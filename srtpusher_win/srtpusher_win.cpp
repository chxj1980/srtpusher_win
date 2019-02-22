// srtpusher_win.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include<string>

using namespace std;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/rational.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
}
#include <thread>

void InitFFmpeg();
void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile);
int OpenCameraVideo(AVFormatContext* pFormatCtx, int *pVideoIndex);
int EncodeVideo(AVFormatContext *pFormatCtx, int videoIndex, string saveFile);
int interrupt_cb(void* cb);

int main()
{
	int videoIndex = -1;
	string saveFile = "D:/fftestvideo.h264";

	InitFFmpeg();
	AVFormatContext *pFormatCtx = avformat_alloc_context();

	//open camera
	if (!OpenCameraVideo(pFormatCtx, &videoIndex))
	{
		cout << "Couldn't open camera." << endl;
		return -1;
	}
	std::thread EncodeThread(EncodeVideo, pFormatCtx, videoIndex, saveFile);
	//std::thread SendThread();
	EncodeThread.join();
	//encode and write to file
	//EncodeVideo(pFormatCtx,videoIndex,saveFile);
}

int OpenCameraVideo(AVFormatContext* pFormatCtx,int *pVideoIndex)
{
	string fileInput = "video=Integrated Camera";//"video=Integrated Webcam";

	pFormatCtx->interrupt_callback.callback = interrupt_cb;
	AVInputFormat *ifmt = av_find_input_format("dshow");
	AVDictionary *format_opts = nullptr;
	av_dict_set_int(&format_opts, "rtbufsize", 18432000, 0);
	av_dict_set(&format_opts, "video_size", "848x480", 0);
	//av_dict_set(&format_opts, "list_options", "true", 0);
	int ret = avformat_open_input(&pFormatCtx, fileInput.c_str(), ifmt, &format_opts);
	if (ret < 0)
	{
		return  ret;
	}
	ret = avformat_find_stream_info(pFormatCtx, nullptr);
	av_dump_format(pFormatCtx, 0, fileInput.c_str(), 0);
	if (ret >= 0)
	{
		cout << "open input stream successfully" << endl;
	}
	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO)
		{
			*pVideoIndex = i;
			break;
		}
	}
	if (*pVideoIndex == -1)
	{
		cout << "Couldn't find a video stream." << endl;
		return -1;
	}
}


int EncodeVideo(AVFormatContext *pFormatCtx, int videoIndex,string saveFile)
{
	//bacause AV_CODEC_ID_H264 means libx264,while libx264 only support yuv420p pix format and libx264rgb can support bgr24 pix format
	AVCodec* pH264Codec = avcodec_find_encoder_by_name("libx264rgb");//avcodec_find_encoder(AVCodecID::AV_CODEC_ID_H264);
	if (!pH264Codec) {
		cout << "Couldn't find the H264 encoder." << endl;
		return -1;
	}
	AVCodecContext* pH264CodecContext = avcodec_alloc_context3(pH264Codec);
	/* put sample parameters */
	pH264CodecContext->bit_rate = 400000;
	/* resolution must be a multiple of two */
	pH264CodecContext->width = pFormatCtx->streams[videoIndex]->codecpar->width;
	pH264CodecContext->height = pFormatCtx->streams[videoIndex]->codecpar->height;
	/* frames per second */
	AVRational r1, r2;
	r1.num = 1;
	r1.den = 25;
	r2.num = 25;
	r2.den = 1;
	pH264CodecContext->time_base = r1;
	pH264CodecContext->framerate = r2;
	//emit one intra frame every ten frames
	pH264CodecContext->gop_size = 10;
	//No b frames for livestreaming
	pH264CodecContext->has_b_frames = 0;
	pH264CodecContext->max_b_frames = 0;
	pH264CodecContext->pix_fmt = AV_PIX_FMT_BGR24;//(AVPixelFormat)pFrame->format;//AV_PIX_FMT_YUV420P;
												  //for livestreaming reduce B frame and make realtime encoding
	av_opt_set(pH264CodecContext->priv_data, "preset", "superfast", 0);
	av_opt_set(pH264CodecContext->priv_data, "tune", "zerolatency", 0);

	/* open it */
	int ret = avcodec_open2(pH264CodecContext, pH264Codec, NULL);
	if (ret < 0) {
		//fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
		exit(1);
	}
	//encode
	AVPacket *pkt = av_packet_alloc();
	if (!pkt)
		exit(1);
	AVFrame *pFrame = av_frame_alloc();

	pFrame->format = pFormatCtx->streams[videoIndex]->codecpar->format;
	pFrame->width = pFormatCtx->streams[videoIndex]->codecpar->width;
	pFrame->height = pFormatCtx->streams[videoIndex]->codecpar->height;

	ret = av_frame_get_buffer(pFrame, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
		exit(1);
	}
	//output file
	FILE* f;
	fopen_s(&f, saveFile.c_str(), "wb");
	if (!f) {
		fprintf(stderr, "Could not open %s\n", "D:/fftestvideo");
		exit(1);
	}
	
	/* encode 1 second of video */
	for (int i = 0; i < 150; i++) {
		//AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
		av_read_frame(pFormatCtx, pkt);
		//判断packet是否为视频帧
		if (pkt->stream_index == videoIndex) {
			//先确定这个packet的codec是啥玩意，然后看看是否需要解码,试了解码不行avopencodec2的时候不行
			//看来不用解码，直接就可以上马
			int size = pkt->size;//equal to 640*480*3 means in bytes
			ret = av_frame_make_writable(pFrame);
			if (ret < 0)
				exit(1);
			//pFrame->data[0] = pkt->data;
			//pFrame->linesize[0] = pFrame->width*3;

			//av_image_fill_linesizes(pFrame->linesize, AV_PIX_FMT_BGR24, 848);
			//av_image_fill_pointers(pFrame->data, AV_PIX_FMT_BGR24, 480, pkt->data, pFrame->linesize);

			pFrame->data[0] = pkt->data + 3 * (pFrame->height - 1) * ((pFrame->width + 3)&~3);
			pFrame->linesize[0] = 3 * ((pFrame->width + 3)&~3) * (-1);
			//调用H264进行编码
			pFrame->pts = i;
			/* encode the image */
			encode(pH264CodecContext, pFrame, pkt, f);
			//放到发送队列，组包TS 通过SRT发送
			av_packet_unref(pkt);
		}
	}

	/* flush the encoder */
	encode(pH264CodecContext, NULL, pkt, f);
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);

	avcodec_free_context(&pH264CodecContext);
	av_frame_free(&pFrame);
	av_packet_free(&pkt);
}

void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile)
{
	int ret;

	/* send the frame to the encoder */
	if (frame)
		printf("Send frame %lld\n", frame->pts);

	ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0) {
		fprintf(stderr, "Error sending a frame for encoding\n");
		exit(1);
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error during encoding\n");
			exit(1);
		}

		printf("Write packet %lld (size=%5d)\n", pkt->pts, pkt->size);
		fwrite(pkt->data, 1, pkt->size, outfile);
		av_packet_unref(pkt);
	}
}

void InitFFmpeg()
{
	avdevice_register_all();
	//deprecated no need to call av_register_all
	//av_register_all();
	//avfilter_register_all(); 
}

int interrupt_cb(void* cb) {
	return 0;
}
void show_dshow_device_option()
{
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options, "list_options", "true", 0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	printf("========Device Option Info======\n");
	avformat_open_input(&pFormatCtx, "video=Integrated Camera", iformat, &options);
	printf("================================\n");
}






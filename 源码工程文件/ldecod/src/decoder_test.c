
/*!
 ***********************************************************************
 *  \file
 *     decoder_test.c
 *  \brief
 *     H.264/AVC decoder test 
 *  \author
 *     Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Yuwen He       <yhe@dolby.com>
 ***********************************************************************
 */

#include "contributors.h"

#include <sys/stat.h>

//#include "global.h"
#include "win32.h"
#include "h264decoder.h"
#include "configfile.h"
#include "direct.h"

#define DECOUTPUT_TEST      0

#define PRINT_OUTPUT_POC    0
#define BITSTREAM_FILENAME  "test.264"
#define DECRECON_FILENAME   "test_dec.yuv"
#define ENCRECON_FILENAME   "test_rec.yuv"
#define FCFR_DEBUG_FILENAME "fcfr_dec_rpu_stats.txt"
#define DECOUTPUT_VIEW0_FILENAME  "H264_Decoder_Output_View0.yuv"
#define DECOUTPUT_VIEW1_FILENAME  "H264_Decoder_Output_View1.yuv"


static void Configure(InputParameters *p_Inp, int ac, char *av[])
{
  //char *config_filename=NULL;
  //char errortext[ET_SIZE];
  memset(p_Inp, 0, sizeof(InputParameters));
  strcpy(p_Inp->infile, BITSTREAM_FILENAME); //! set default bitstream name
  strcpy(p_Inp->outfile, DECRECON_FILENAME); //! set default output file name
  strcpy(p_Inp->reffile, ENCRECON_FILENAME); //! set default reference file name
  
#ifdef _LEAKYBUCKET_
  strcpy(p_Inp->LeakyBucketParamFile,"leakybucketparam.cfg");    // file where Leaky Bucket parameters (computed by encoder) are stored
#endif

  ParseCommand(p_Inp, ac, av);

  fprintf(stdout,"----------------------------- JM %s %s -----------------------------\n", VERSION, EXT_VERSION);
  //fprintf(stdout," Decoder config file                    : %s \n",config_filename);
  if(!p_Inp->bDisplayDecParams)
  {
    fprintf(stdout,"--------------------------------------------------------------------------\n");
    fprintf(stdout," Input H.264 bitstream                  : %s \n",p_Inp->infile);
    fprintf(stdout," Output decoded YUV                     : %s \n",p_Inp->outfile);
    //fprintf(stdout," Output status file                     : %s \n",LOGFILE);
    fprintf(stdout," Input reference file                   : %s \n",p_Inp->reffile);

    fprintf(stdout,"--------------------------------------------------------------------------\n");
  #ifdef _LEAKYBUCKET_
    fprintf(stdout," Rate_decoder        : %8ld \n",p_Inp->R_decoder);
    fprintf(stdout," B_decoder           : %8ld \n",p_Inp->B_decoder);
    fprintf(stdout," F_decoder           : %8ld \n",p_Inp->F_decoder);
    fprintf(stdout," LeakyBucketParamFile: %s \n",p_Inp->LeakyBucketParamFile); // Leaky Bucket Param file
    calc_buffer(p_Inp);
    fprintf(stdout,"--------------------------------------------------------------------------\n");
  #endif
  }
  
}

/*********************************************************
if bOutputAllFrames is 1, then output all valid frames to file onetime; 
else output the first valid frame and move the buffer to the end of list;
*********************************************************/
static int WriteOneFrame(DecodedPicList *pDecPic, int hFileOutput0, int hFileOutput1, int bOutputAllFrames)
{
  int iOutputFrame=0;
  DecodedPicList *pPic = pDecPic;

  if(pPic && (((pPic->iYUVStorageFormat==2) && pPic->bValid==3) || ((pPic->iYUVStorageFormat!=2) && pPic->bValid==1)) )
  {
    int i, iWidth, iHeight, iStride, iWidthUV, iHeightUV, iStrideUV;
    byte *pbBuf;    
    int hFileOutput;
    int res;

    iWidth = pPic->iWidth*((pPic->iBitDepth+7)>>3);
    iHeight = pPic->iHeight;
    iStride = pPic->iYBufStride;
    if(pPic->iYUVFormat != YUV444)
      iWidthUV = pPic->iWidth>>1;
    else
      iWidthUV = pPic->iWidth;
    if(pPic->iYUVFormat == YUV420)
      iHeightUV = pPic->iHeight>>1;
    else
      iHeightUV = pPic->iHeight;
    iWidthUV *= ((pPic->iBitDepth+7)>>3);
    iStrideUV = pPic->iUVBufStride;
    
    do
    {
      if(pPic->iYUVStorageFormat==2)
        hFileOutput = (pPic->iViewId&0xffff)? hFileOutput1 : hFileOutput0;
      else
        hFileOutput = hFileOutput0;
      if(hFileOutput >=0)
      {
        //Y;
        pbBuf = pPic->pY;
        for(i=0; i<iHeight; i++)
        {
          res = write(hFileOutput, pbBuf+i*iStride, iWidth);
          if (-1==res)
          {
            error ("error writing to output file.", 600);
          }
        }

        if(pPic->iYUVFormat != YUV400)
        {
         //U;
         pbBuf = pPic->pU;
         for(i=0; i<iHeightUV; i++)
         {
           res = write(hFileOutput, pbBuf+i*iStrideUV, iWidthUV);
           if (-1==res)
           {
             error ("error writing to output file.", 600);
           }
		 }
         //V;
         pbBuf = pPic->pV;
         for(i=0; i<iHeightUV; i++)
         {
           res = write(hFileOutput, pbBuf+i*iStrideUV, iWidthUV);
           if (-1==res)
           {
             error ("error writing to output file.", 600);
           }
         }
        }

        iOutputFrame++;
      }

      if (pPic->iYUVStorageFormat == 2)
      {
        hFileOutput = ((pPic->iViewId>>16)&0xffff)? hFileOutput1 : hFileOutput0;
        if(hFileOutput>=0)
        {
          int iPicSize =iHeight*iStride;
          //Y;
          pbBuf = pPic->pY+iPicSize;
          for(i=0; i<iHeight; i++)
          {
            res = write(hFileOutput, pbBuf+i*iStride, iWidth);
            if (-1==res)
            {
              error ("error writing to output file.", 600);
            }
          }

          if(pPic->iYUVFormat != YUV400)
          {
           iPicSize = iHeightUV*iStrideUV;
           //U;
           pbBuf = pPic->pU+iPicSize;
           for(i=0; i<iHeightUV; i++)
           {
             res = write(hFileOutput, pbBuf+i*iStrideUV, iWidthUV);
             if (-1==res)
             {
               error ("error writing to output file.", 600);
             }
           }
           //V;
           pbBuf = pPic->pV+iPicSize;
           for(i=0; i<iHeightUV; i++)
           {
             res = write(hFileOutput, pbBuf+i*iStrideUV, iWidthUV);
             if (-1==res)
             {
               error ("error writing to output file.", 600);
             }
           }
          }

          iOutputFrame++;
        }
      }

#if PRINT_OUTPUT_POC
      fprintf(stdout, "\nOutput frame: %d/%d\n", pPic->iPOC, pPic->iViewId);
#endif
      pPic->bValid = 0;
      pPic = pPic->pNext;
    }while(pPic != NULL && pPic->bValid && bOutputAllFrames);
  }
#if PRINT_OUTPUT_POC
  else
    fprintf(stdout, "\nNone frame output\n");
#endif

  return iOutputFrame;
}

/*!
 ***********************************************************************
 * \brief
 *    main function for JM decoder
 ***********************************************************************
 */
int main(int argc, char **argv)
{
  int iRet;
  int i;
  DecodedPicList *pDecPicList;
  int hFileDecOutput0=-1, hFileDecOutput1=-1;
  int iFramesOutput=0, iFramesDecoded=0;
  InputParameters InputParams;
  char essRoute[255];

  cannySize = 5;
  maxThresold = 150;
  minthresold = 50;

  system("cls");	//清屏幕

  //输出自己的版权
  printf("|*********************************************************************************|\n");
  printf("|*                                     版权归属                                  *|\n");
  printf("|*                   华南理工大学  计算机科学与工程学院  梁剑                    *|\n");
  printf("|*                              E-mail:doc.liang@qq.com                          *|\n");
  printf("|*                                 仅供学习研究用途                              *|\n");
  printf("|*********************************************************************************|\n\n");

  yuv == NULL;
  frame_id = 0;
  sr = 8;
  fps = 1000/40;

  for(i = 1;i<argc;i++){		//读超分的倍数
	  if (0 == strcmp(argv[i],"-sr")){
		  ////////////////得到sr的倍率
		  if(argv[i+1] != NULL && atoi(argv[i+1])>0)
			  sr = atoi(argv[i+1]);
		  else{
			  printf("wrong with super resolution times input!\n");
			  exit(-1);
		  }

		  /////////////////后处理
		  for(i=i+2;i<argc;i++)
			  argv[i-2] = argv[i];
		  argc = argc - 2;

		  break;
	  }
  }
  for(i = 1;i<argc;i++){		//读fps
	  if (0 == strcmp(argv[i],"-fps")){
		  ////////////////得到播放的fps
		  if(argv[i+1] != NULL && atoi(argv[i+1])>0)
			  fps = 1000 / atoi(argv[i+1]);
		  else{
			  printf("wrong with fps input!\n");
			  exit(-1);
		  }

		  /////////////////后处理
		  for(i=i+2;i<argc;i++)
			  argv[i-2] = argv[i];
		  argc = argc - 2;

		  break;
	  }
  }
  for(i = 1;i<argc;i++){		//yuv路径，方便计算ESS
	  if (0 == strcmp(argv[i],"-ESS")){
		  ////////////////得到sr的倍率
		  if(argv[i+1] != NULL){
			  yuv = fopen(argv[i+1],"rb");
			  if(yuv == NULL){
				  printf("wrong with yuv direction !\n");
				  exit(-1);
			  }
		  }else{
			  printf("wrong with yuv direction !\n");
			  exit(-1);
		  }

		  /////////////////后处理
		  for(i=i+2;i<argc;i++)
			  argv[i-2] = argv[i];
		  argc = argc - 2;

		  break;
	  }
  }

  for(i = 1;i<argc;i++){		//canny检测器参数
	  if (0 == strcmp(argv[i],"-canny")){

		  ////////////////得到sr的倍率
		  if(argv[i+1] != NULL && argv[i+2] != NULL && argv[i+3] != NULL){
			  minthresold = atoi(argv[i+1]);
			  maxThresold = atoi(argv[i+2]);
			  cannySize = atoi(argv[i+3]);
		  }else{
			  printf("wrong with yuv direction !\n");
			  exit(-1);
		  }

		  /////////////////后处理
		  for(i=i+4;i<argc;i++)
			  argv[i-4] = argv[i];
		  argc = argc - 4;

		  break;
	  }
  }

#if DECOUTPUT_TEST
  hFileDecOutput0 = open(DECOUTPUT_VIEW0_FILENAME, OPENFLAGS_WRITE, OPEN_PERMISSIONS);
  fprintf(stdout, "Decoder output view0: %s\n", DECOUTPUT_VIEW0_FILENAME);
  hFileDecOutput1 = open(DECOUTPUT_VIEW1_FILENAME, OPENFLAGS_WRITE, OPEN_PERMISSIONS);
  fprintf(stdout, "Decoder output view1: %s\n", DECOUTPUT_VIEW1_FILENAME);
#endif

  init_time();

  //get input parameters;
  Configure(&InputParams, argc, argv);

  //建立sketch图片路径以及更改yuv文件输出名字
  strcpy(route,InputParams.infile);
  route[strlen(route)-4] = '\0';
  strcpy(InputParams.outfile,route);
  strcat(InputParams.outfile,".yuv");
  strcat(route,"_sketchPic");
  _mkdir(route);		//在当前目录下创建文件夹以存放攻击后的图片

  //打开写ess指标的文件
  if(yuv != NULL){
	  strcpy(essRoute,InputParams.infile);
	  essRoute[strlen(essRoute)-4] = '\0';
	  strcat(essRoute,"_ESS.csv");
	  essFile = fopen(essRoute,"wb");
	  fprintf(essFile,"frameNo,slice_type,ESS\n");
	  mean = 0.0;
  }

  //open decoder;
  iRet = OpenDecoder(&InputParams);

  if(iRet != DEC_OPEN_NOERR)
  {
    fprintf(stderr, "Open encoder failed: 0x%x!\n", iRet);
    return -1; //failed;
  }

  //decoding;
  do
  {
    iRet = DecodeOneFrame(&pDecPicList);

    if(iRet==DEC_EOS || iRet==DEC_SUCCEED)
    {
      //process the decoded picture, output or display;
      iFramesOutput += WriteOneFrame(pDecPicList, hFileDecOutput0, hFileDecOutput1, 0);
      iFramesDecoded++;
    }
    else
    {
      //error handling;
      fprintf(stderr, "Error in decoding process: 0x%x\n", iRet);
    }
  }while((iRet == DEC_SUCCEED) && ((p_Dec->p_Inp->iDecFrmNum==0) || (iFramesDecoded<p_Dec->p_Inp->iDecFrmNum)));

  iRet = FinitDecoder(&pDecPicList);
  iFramesOutput += WriteOneFrame(pDecPicList, hFileDecOutput0, hFileDecOutput1 , 1);
  iRet = CloseDecoder();

  //quit;
  if(hFileDecOutput0>=0)
  {
    close(hFileDecOutput0);
  }
  if(hFileDecOutput1>=0)
  {
    close(hFileDecOutput1);
  }

  printf("%d frames are decoded.\n", iFramesDecoded);

  if(yuv != NULL){
	  //写mean
	  fprintf(essFile,"\nmean,,%f\n",mean/frame_id);
	fclose(yuv);
	fclose(essFile);
  }

  return 0;
}



#include "T2FGMM_UV.h"

T2FGMM_UV::T2FGMM_UV() : firstTime(true), frameNumber(0), showOutput(true), threshold(9.0), alpha(0.01), gaussians(3), km(1.5), kv(0.6)
{
  std::cout << "T2FGMM_UV()" << std::endl;
}

T2FGMM_UV::~T2FGMM_UV()
{
  std::cout << "~T2FGMM_UV()" << std::endl;
}

void T2FGMM_UV::process(const cv::Mat &img_input, cv::Mat &img_output)
{
  if(img_input.empty())
    return;

  loadConfig();

  if(firstTime)
    saveConfig();

  frame = new IplImage(img_input);
  
  if(firstTime)
    frame_data.ReleaseMemory(false);
  frame_data = frame;

  if(firstTime)
  {
    int width	= img_input.size().width;
    int height = img_input.size().height;

    lowThresholdMask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    lowThresholdMask.Ptr()->origin = IPL_ORIGIN_BL;

    highThresholdMask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    highThresholdMask.Ptr()->origin = IPL_ORIGIN_BL;

    params.SetFrameSize(width, height);
    params.LowThreshold() = threshold;
    params.HighThreshold() = 2*params.LowThreshold();
    params.Alpha() = alpha;
    params.MaxModes() = gaussians;
    params.Type() = TYPE_T2FGMM_UV;
    params.KM() = km; // Factor control for the T2FGMM-UM [0,3] default: 1.5
    params.KV() = kv; // Factor control for the T2FGMM-UV [0.3,1] default: 0.6

    bgs.Initalize(params);
    bgs.InitModel(frame_data);
  }

  bgs.Subtract(frameNumber, frame_data, lowThresholdMask, highThresholdMask);
  lowThresholdMask.Clear();
  bgs.Update(frameNumber, frame_data, lowThresholdMask);
  
  cv::Mat foreground(highThresholdMask.Ptr());

  //if(showOutput)
    //cv::imshow("T2FGMM-UV", foreground);
  
  foreground.copyTo(img_output);

  delete frame;
  firstTime = false;
  frameNumber++;
}

void T2FGMM_UV::saveConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/T2FGMM_UV.xml", 0, CV_STORAGE_WRITE);

  cvWriteReal(fs, "threshold", threshold);
  cvWriteReal(fs, "alpha", alpha);
  cvWriteReal(fs, "km", km);
  cvWriteReal(fs, "kv", kv);
  cvWriteInt(fs, "gaussians", gaussians);
  cvWriteInt(fs, "showOutput", showOutput);

  cvReleaseFileStorage(&fs);
}

void T2FGMM_UV::loadConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/T2FGMM_UV.xml", 0, CV_STORAGE_READ);
  
  threshold = cvReadRealByName(fs, 0, "threshold", 9.0);
  alpha = cvReadRealByName(fs, 0, "alpha", 0.01);
  km = cvReadRealByName(fs, 0, "km", 1.5);
  kv = cvReadRealByName(fs, 0, "kv", 0.6);
  gaussians = cvReadIntByName(fs, 0, "gaussians", 3);
  showOutput = cvReadIntByName(fs, 0, "showOutput", true);

  cvReleaseFileStorage(&fs);
}
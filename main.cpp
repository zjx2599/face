#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <opencv2/face/facerec.hpp>
#include <opencv2/face.hpp>

#define NEED_TO_CREAT_FILE
#define NEED_PREDICT

using namespace std;
using namespace cv;
using namespace cv::face;

int get_file_name(string path);

string haar_face_datapath = "./haarcascades/haarcascade_frontalface_alt_tree.xml";

/**
 * @brief main function
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
#ifndef NEED_TO_CREAT_FILE
      get_file_name("./orl_faces");
#endif

      VideoCapture capture(0);
      if (!capture.isOpened())
      {
            printf("could not open camera...\n");
            return -1;
      }

      CascadeClassifier faceDetector;
      faceDetector.load(haar_face_datapath);
      vector<Rect> face_rects;
      int count = 0;

      Mat frame;
      namedWindow("camera", CV_WINDOW_AUTOSIZE);
      while (capture.read(frame))
      {
            flip(frame, frame, 1);
            faceDetector.detectMultiScale(frame, face_rects, 1.01, 1, 0, Size(100, 100), Size(400, 400));
            for (int i = 0; i < face_rects.size(); i++)
            {
                  rectangle(frame, face_rects[i], Scalar(255, 0, 0), 2, 8, 0);
                  if (count % 10 == 0)
                  {
                        Mat dst;
                        resize(frame(face_rects[i]), dst, Size(100, 100));
                        imshow("face", dst);
                        imwrite(format("./sample/zjx/face_%d.jpg", count), dst);
                        cout << count << endl;
                  }
            }
            imshow("camera", frame);
            char c = waitKey(10);
            if (c == 27)
            {
                  break;
            }
            count++;
      }

      capture.release();

#ifndef NEED_PREDICT
      string filename = string("./orl_faces/data.txt");
      ifstream file(filename.c_str(), ios::in);
      if (!file)
      {
            printf("could not load file correctly...\n");
            return -1;
      }

      string line, path, classlable;
      vector<Mat> images;
      vector<int> labels;

      char separator = ';';
      while (getline(file, line))
      {
            stringstream sstr(line);
            getline(sstr, path, separator);
            getline(sstr, classlable);
            if (!path.empty() && !classlable.empty())
            {
                  images.push_back(imread(path, 0));
                  labels.push_back(atoi(classlable.c_str()));
            }
      }

      if (images.size() < 1 || labels.size() < 1)
      {
            printf("invalid image path...\n");
            return -1;
      }

      int height = images[0].rows;
      int width = images[0].cols;
      printf("height : %d , width : %d \n", height, width);

      Mat testSample = images[images.size() - 1];
      int testLabel = labels[labels.size() - 1];
      images.pop_back();
      labels.pop_back();

      /**
       * @brief train
       */
      Ptr<BasicFaceRecognizer> model = cv::face::createEigenFaceRecognizer();
      model->train(images, labels);

      /**
       * @brief use sample to test
       */
      int predictedLabel = model->predict(testSample);
      printf("actual label : %d  predicted label %d\n", testLabel, predictedLabel);
#endif

      return 0;
}

/**
 * @brief This function produce path file
 * 
 * @param path 
 * @return int 
 */
int get_file_name(string path)
{
      DIR *dir;
      dirent *p;
      if ((dir = opendir(path.c_str())) == NULL)
      {
            printf("dir_order: can't open %s\n", path.c_str());
            return -1;
      }
      while ((p = readdir(dir)) != NULL)
      {
            if (p->d_name[0] == '.')
                  continue;
            string subpath;
            subpath = path + '/' + (string)(p->d_name);

            struct stat filestat;
            if (stat(subpath.c_str(), &filestat) == -1)
            {
                  printf("cannot access the file %s\n", subpath.c_str());
                  return -1;
            }
            if ((filestat.st_mode & S_IFMT) == S_IFDIR)
            {
                  get_file_name(subpath);
            }
            else
            {
                  string num = subpath.substr(13, 2);
                  if (num.substr(1, 2) == "/")
                        num = num.substr(0, 1);
                  ofstream outfile;
                  outfile.open("./orl_faces/data.txt", ios::app);
                  outfile << subpath << ";" << num << endl;
                  outfile.close();
            }
      }
      closedir(dir);
      return 0;
}

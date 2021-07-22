/*=================================================================
 * Extract Background & Foreground Model by ViBe Algorithm using OpenCV Library.
 *
 * Copyright (C) 2017 Chandler Geng. All rights reserved.
 *
 *     This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *     You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA
===================================================================
*/

#include "Vibe.h"
#include "validation.h"

bool white_sum(Mat *img, int threshold){
    int counter= 0;
    Mat_<uchar>::iterator it = img->begin<uchar>();
    Mat_<uchar>::iterator end = img->end<uchar>();
    for(; it != end; it++){
        if (*it == 255) counter ++;
    }
    if (counter >= threshold) return true;
    else return false;
}

int main(int argc, char* argv[])
{
    Mat frame, gray, FGModel, frame2;
    VideoCapture capture;
    capture = VideoCapture(R"(C:\Users\stephen.gao\Desktop\c\test01.avi)");
    if(!capture.isOpened()) {
        cout << "ERROR: Didn't find this video!" << endl;
        return 0;
    }

    if (!capture.isOpened())
    {
        cout << "No camera or video input!" << endl;
        return -1;
    }

    ViBe vibe;
    bool count = true;
    bool indicator = false;
    int frame_num = 1;
    int *stds = new int[4]{0};
    vector<int> fp, fn;

    while (1)
    {
        capture >> frame;
        if (frame.empty())
            continue;

        cvtColor(frame(Rect(120, 0, 520, 480)), gray, COLOR_RGB2GRAY);
        if (count)
        {
            vibe.init(gray);
//            capture >> frame2;
//            capture >> frame3;
//            cvtColor(frame2(Rect(120, 0, 520, 480)), gray2, COLOR_RGB2GRAY);
//            cvtColor(frame3(Rect(120, 0, 520, 480)), gray3, COLOR_RGB2GRAY);
//            vibe.ProcessFirstFewFrames(gray, gray2, gray3);
            vibe.ProcessFirstFrame(gray);
            cout<<"Training ViBe Success."<<endl;
            count=false;
        }
        else
        {
            vibe.Run(gray);
            FGModel = vibe.getFGModel();
			morphologyEx(FGModel, FGModel, MORPH_OPEN, Mat());
            imshow("FGModel", FGModel);
            imshow("input", frame(Rect(120, 0, 520, 480)));

            /* 判断是否为空:  */
            for (int i = 0; i < 480; i += 40) {
                indicator = false;
                for (int j = 0; j < 440; j += 40) {
                    Mat square = FGModel(Rect(i, j, 40, 40));
                    if (white_sum(&square, 100)) {
                        indicator = true;
                        validate_01(frame_num, stds, &fp, &fn, indicator);
//                        validate_02(frame_num, stds, &fp, &fn, indicator);
//                        validate_03(frame_num, stds, &fp, &fn, indicator);
                        break;
                    }
                }
                if (indicator) break;
            }
            if (!indicator){
                validate_01(frame_num, stds, &fp, &fn);
//                validate_02(frame_num, stds, &fp, &fn);
//                validate_03(frame_num, stds, &fp, &fn);
            }
        }

//        if (frame_num >= 2942 && frame_num <= 5708) {
//            cout << "required frame reached." << endl;
//            if (waitKey(0) == 27) {
//                frame_num ++;
//                continue;
//            }
//        }

        if (frame_num == capture.get(7)) {
            destroyAllWindows();
            break;
        }

        if (waitKey(1) == 27) {
//            imwrite(R"(C:\Users\stephen.gao\Desktop\c\background.jpg)", frame);
            break;
        }

        frame_num ++;
    }

    cout << "The TP is " << stds[0] << endl;
    cout << "The TN is " << stds[1] << endl;
    cout << "The FP is " << stds[2] << endl;
    cout << "The FN is " << stds[3] << endl;
    for (int & it : fn){
        cout << "FN frame number is " << it << endl;
    }
    for (int & it : fp){
        cout << "FP frame number is " << it << endl;
    }

    delete[] stds;

    return 0;
}

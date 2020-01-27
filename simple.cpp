#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <fstream>
#include <iostream>
#include <string>

struct ArgumentList {
	std::string image_name;		
	int wait_t;                  
};

bool ParseInputs(ArgumentList& args, int argc, char **argv) {

	if(argc<3 || (argc==2 && std::string(argv[1]) == "--help") || (argc==2 && std::string(argv[1]) == "-h") || (argc==2 && std::string(argv[1]) == "-help")) {
		std::cout << "usage: simple -i <image_name>" << std::endl;
		std::cout << "exit:  type q" << std::endl << std::endl;
		std::cout << "Allowed options:" << std::endl <<
				"   -h	                     produce help message" << std::endl <<
				"   -i arg                   image name. Use %06d format for multiple images" << std::endl <<
				"   -t arg                   wait before next frame (ms) *default = 0*" << std::endl << std::endl << std::endl;
				
		return false;
	}

	int i = 1;
	while(i < argc) {
		if(std::string(argv[i]) == "-i") {
			args.image_name = std::string(argv[++i]);
		}
		if(std::string(argv[i]) == "-t") {
			args.wait_t = atoi(argv[++i]);
		}
		else
			args.wait_t = 0;
		++i;
	}

	return true;
}

//calc foreground 
void fgdiff(const cv::Mat& background, cv::Mat& fg, unsigned char threshold) {
	for(int ii = 0; ii < fg.rows; ++ii) {
		for(int jj = 0; jj < fg.cols; ++jj) {
			unsigned char& f = fg.data[(ii*fg.cols+jj)*fg.elemSize()];
			unsigned char b = background.data[(ii*background.cols+jj)*background.elemSize()];
			if(std::abs(int(f) - int(b)) <= threshold) {
				f = 0; 
			}
		}
	}
}

int main(int argc, char **argv)
{
	int frame_number = 0;
	char frame_name[256];
	bool exit_loop = false;

	//parse argument list
	ArgumentList args;
	if(!ParseInputs(args, argc, argv)) {
		return 1;
	}

	//prev frame
	cv::Mat m_pframe_bg;

	//exp average
	cv::Mat m_exp_av_bg;
	float m_alpha = 0.5;

	//moving average
	std::vector<cv::Mat> m_f_history;
	std::vector<cv::Mat> m_f_history_float;
	cv::Mat m_ma_bg;

	//background 2.0
	cv::Mat m_ma_bg_2;
	m_ma_bg_2 = cv::Scalar(0.0f);
	const unsigned int m_N = 10;
	unsigned char m_fg_threshold = 50;

	while(!exit_loop)
	{
		//multi frame case
		if(args.image_name.find('%') != std::string::npos)
			sprintf(frame_name,(const char*)(args.image_name.c_str()),frame_number);
		else 
			sprintf(frame_name,"%s",args.image_name.c_str());

		//opening file
		std::cout << "Opening.." << frame_name << std::endl;

		cv::Mat image = cv::imread(frame_name, CV_8UC1);
		if(image.empty()) {
			std::cout<<"Unable to open"<<frame_name<<std::endl;
			return 1;
		}

		//prev frame
		if(m_pframe_bg.empty()) { 
			m_pframe_bg = image;
		} else {
			//we have a bg
			cv::Mat fg = image.clone();
			fgdiff(m_pframe_bg, fg, m_fg_threshold); //fg calc

			cv::namedWindow("PrevFrame-bg", CV_WINDOW_NORMAL);
			cv::namedWindow("PrevFrame", CV_WINDOW_NORMAL);
			cv::imshow("PrevFrame-bg", m_pframe_bg);
			cv::imshow("PrevFrame", fg);
			
			//update bg
			m_pframe_bg = image;
		}

		//exp avg
		if(m_exp_av_bg.empty()) { 
			m_exp_av_bg = image; 
		} else {
			//we have a bg
			cv::Mat fg = image.clone();
			fgdiff(m_exp_av_bg, fg, m_fg_threshold); //fg calc

			cv::namedWindow("ExpAverage", CV_WINDOW_NORMAL);
			cv::namedWindow("ExpAverage-BG", CV_WINDOW_NORMAL);
			cv::imshow("ExpAverage-BG", m_exp_av_bg);
			cv::imshow("ExpAverage", fg);
			
			//update bg
			for(int ii = 0; ii < m_exp_av_bg.rows; ++ii) {
				for(int jj = 0; jj < m_exp_av_bg.cols; ++jj) {
					unsigned char& f = m_exp_av_bg.data[(ii*fg.cols+jj)*fg.elemSize()]; 
					unsigned char b = image.data[(ii*image.cols+jj)*image.elemSize()]; 
					f = static_cast<unsigned char>(m_alpha*f + (1-m_alpha)*b);
				}
			}
		}

		//mov avg
		if(m_f_history.size() < m_N) {
			m_f_history.push_back(image);
		} else {
			if(m_ma_bg.empty()) {
				m_ma_bg.create(image.rows, image.cols, CV_32FC1);
				m_ma_bg = cv::Scalar(0.0f);

				for(unsigned int ii = 0; ii < m_f_history.size(); ++ii) {
					for(int u = 0; u < image.cols; ++u) {
						for(int v = 0; v < image.rows; ++v) {
							float * bg = (float *) (m_ma_bg.data + (v*image.cols + u) * m_ma_bg.elemSize());
							float hi = m_f_history[ii].data[(v*image.cols + u)];
							*bg = *bg + hi;
						}
					}
				}

				m_ma_bg /= m_N;
			}

			//patch
			cv::Mat m_exp_av_bg_uchar(image.rows, image.cols, CV_8UC1);
			m_ma_bg.convertTo(m_exp_av_bg_uchar, CV_8UC1);

			cv::Mat fg = image.clone();
			fgdiff(m_exp_av_bg_uchar, fg, m_fg_threshold);

			cv::namedWindow("MovAverage", CV_WINDOW_NORMAL);
			cv::namedWindow("MovAverage-bg", CV_WINDOW_NORMAL);
			imshow("MovAverage-bg", m_exp_av_bg_uchar);
			imshow("MovAverage", fg);

			//adjust the background
			m_ma_bg *= m_N;

			for(int u = 0; u < image.cols; ++u) {
				for(int v = 0; v < image.rows; ++v) {
					float * bg = (float *)(m_ma_bg.data + (v*image.cols + u) * m_ma_bg.elemSize());
					float hi = m_f_history.front().data[(v*image.cols + u)];
					*bg = *bg - hi;
				}
			}

			for(int u = 0; u < image.cols; ++u) {
				for(int v = 0; v < image.rows; ++v) {
					float * bg = (float *)(m_ma_bg.data + (v*image.cols + u) * m_ma_bg.elemSize());
					float in = image.data[(v*image.cols + u)];
					*bg = *bg + in;
				}
			}

			m_ma_bg /= m_N;

			//adjust the buffer
			m_f_history.push_back(image);
			m_f_history.erase(m_f_history.begin());
		}

		//display image
		cv::namedWindow("image", cv::WINDOW_NORMAL);
		cv::imshow("image", image);

		//wait for key or timeout
		unsigned char key = cv::waitKey(args.wait_t);
		std::cout << "key " << int(key) << std::endl;

		if(key == 'q')
			exit_loop = true;

		frame_number++;
	}

	return 0;
}

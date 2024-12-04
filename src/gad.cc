#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

// for examples kPI, sin/cos
#include <cmath>
// for accumulate
#include <numeric>
// for benchmarking
#include <ctime>

#include "drawille.hpp"
#include <locale>

int main(int argc, char* argv[]){
	
	std::string thepipe="/tmp/gadpipe";
	if(argc >1){
		thepipe=argv[1];
	}
	
	// plot size in lines x chars
	int c_height=20;
	int c_width=80;
	char* tmp=nullptr;
	tmp=getenv("LINES");
	//printf("LINES is %p\n",tmp);
	if(tmp) c_height=std::atoi(tmp);
	tmp=nullptr;
	tmp=getenv("COLUMNS");
	//printf("COLUMNS is %p\n",tmp);
	if(tmp) c_width=std::atoi(tmp);
	// account for axes. Acually drawille doesn't plot these. TODO
	c_height -= 3;
	//c_width -= 2;
	std::clog<<"canvas size: "<<c_width<<"x"<<c_height<<std::endl;
	
	// required by drawille
	std::locale::global(std::locale(""));
	Drawille::Canvas canvas(c_width, c_height);
	
	// braille chars are 2 wide and 4 heigh, so we can have more points than actual widthxheight
	int width = c_width * 2.;
	int height = c_height *4;
	double datamax=0;
	
	// XXX note: using stdout (cout or printf) will corrupt the plot
	// we can use std::clog safely
	
	while(true){
		// open the pipe
		std::clog<<"Reading from "<<thepipe<<std::endl;
		std::ifstream fs(thepipe);
		while(!fs.is_open()){
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			fs.open(thepipe);
		}
		std::clog<<"fifo opened"<<std::endl;
		
		bool first=true;
		std::vector<double> nums;
		std::vector<double> nums2;
		std::string newline;
		
		while(fs.is_open()){
			
			// try to get next line
			std::getline(fs,newline);
			if(!fs.good() || newline.empty()){
				std::clog<<"hit EOF, closing"<<std::endl;
				fs.close();
				break;
			}
			
			// parse as a series of numbers
			std::stringstream ss(newline);
			double nextnum;
			nums.resize(0); // dispose of elements (without resetting capacity)
			while(ss >> nextnum) nums.push_back(nextnum);
			
			// we have 'canvas.unset(x,y)', but it's more efficient to just replace it
			std::clock_t clear_start = std::clock();
			if(!first){
				canvas = Drawille::Canvas(c_width, c_height);
			}
			clock_t clear_end = std::clock();
			first=false;
			
			// scale x range to fit window. Our plottable range is 0 to width inclusive.
			if(nums.size()>width+1){
				// this simple averaging method can only scale down by integer factors,
				// so if we have a width of 160 and 161 points, we scale down to just 80.
				// TODO better solution?
				int windowsize=ceil(double(nums.size())/double(width));
				//std::clog<<"scaling x range by factor "<<windowsize<<std::endl;
				nums2.resize(0);
				for(int i=0; i<floor(nums.size()/windowsize); ++i){
					int lower=i*windowsize;
					int upper=std::min(nums.size(),size_t((i+1)*windowsize))-1;
					double windowavg = std::accumulate(nums.begin()+lower,nums.begin()+upper,0)/windowsize;
					nums2.push_back(windowavg);
				}
				std::swap(nums,nums2);
			}
			//std::clog<<"we have "<<nums.size()<<" points"<<std::endl;
			
			// scale y values. Our plottable range is much less than our true range so this is required.
			// but we may not always want to have a normalised plot (to more easily see scale changes)
			// so scale to the highest value we've seen.
			// while we're at it subtract off any baseline offset
			double thismax = *std::max_element(nums.begin(),nums.end());
			double thismin = *std::min_element(nums.begin(),nums.end());
			if((thismax-thismin)>datamax) datamax=(thismax-thismin);
			for(size_t i=0; i<nums.size(); ++i){
				nums[i] = height*((nums[i]-thismin)/datamax);
				// drawing out of range crashes the application!
				// range is 1 to (height-1): CANNOT PLOT 0 OR HEIGHT
				if(nums[i]>=height){
					//std::clog<<nums[i]<<" is > than height, clipping to "<<(height-1)<<std::endl;
					nums[i]=height-1;
				} else if(nums[i]<1){
					//std::clog<<"coercing <1 to 1"<<std::endl;
					nums[i] = 1;
				}
			}
			
			std::clock_t write_start = std::clock();
			for(size_t i=0; i<nums.size(); ++i){
				// technically y values are from top down, so need to be given as ymax-value
				//std::clog<<"{"<<i<<","<<height-nums[i]<<"}"<<std::endl;
				canvas.set(i, height-nums[i]);
			}
			std::clock_t write_end = std::clock();
			canvas.draw(std::wcout);
			std::clock_t draw_end = std::clock();
			
			/*
			std::clog<<"clear took "   <<(clear_end-clear_start)*1./CLOCKS_PER_SEC
			         <<", write took " <<(write_end-write_start)*1./CLOCKS_PER_SEC
			         <<", draw took "  <<(draw_end-write_end)*1./CLOCKS_PER_SEC
			         <<", total "      <<(write_end-clear_start)*1./CLOCKS_PER_SEC<<std::endl;
			*/
			//fs.close();  // XXX DEBUG XXX
		}
		
		std::clog<<"FIFO closed, re-opening in 5s..."<<std::endl;
		//break; // XXX DEBUG XXX
		first=true;
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	}
	
	
	return 0;
}

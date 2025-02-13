#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <fstream>

// for examples kPI, sin/cos
#include <cmath>
// for accumulate
#include <numeric>
// for benchmarking
#include <ctime>

#include "ascii.h"

void example() {
	using namespace ascii;
	std::vector<double> series;
	std::vector<double> series2;
	// height is number of lines it should take up,
	// it will scale data to fit
	int height = 6;
	for (int i = 0; i < 100; i += 2) {
		series.push_back(15 * std::cos(i * (kPI * 8) / 120));
		series2.push_back(15 * std::sin(i * ((kPI * 4) / 100)));
		// the constructor takes a vector of series', each being a vector of values
		ascii::Asciichart asciichart(std::vector<std::vector<double>>{series, series2});
		// alternatively to specify a name for your series, pass a vector of pairs like so:
		//Asciichart asciichart({{"A", series}, {"B", series2}});
		if (i != 0) {
			for (int j = 0; j <= height; j++) {
				std::cout << "\033[A\033[2K"; // This is used for clear previous chart
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		std::cout << asciichart.type(Asciichart::LINE)
		                       .show_legend(true) // add this to enable legend
		                       .height(height)
		                       .Plot();
	}
}

int main(int argc, char* argv[]){
	
	std::string thepipe="/tmp/gadpipe";
	if(argc >1){
		thepipe=argv[1];
	}
	
	// plot size in lines x chars
	int height=45;
	int width=140;
	char* tmp=nullptr;
	tmp=getenv("LINES");
	if(tmp) height=std::atoi(tmp);
	tmp=nullptr;
	tmp=getenv("COLUMNS");
	if(tmp) width=std::atoi(tmp);
	// eehh safety factors, it doesn't seem to line to really filly it.
	height -= 5;
	width -= 10; // FIXME increase if legend being shown
	std::cout<<"width: "<<width<<", height: "<<height<<std::endl;
	
	while(true){
		// open the pipe
		std::cout<<"Reading from "<<thepipe<<std::endl;
		std::ifstream fs(thepipe);
		while(!fs.is_open()){
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			fs.open(thepipe);
		}
		std::cout<<"fifo opened"<<std::endl;
		
		bool first=true;
		std::vector<double> nums;
		std::vector<double> nums2;
		std::string newline;
		
		while(fs.is_open()){
			
			// try to get next line
			std::getline(fs,newline);
			if(!fs.good()){
				fs.close();
				break;
			}
			
			// parse as a series of numbers
			std::stringstream ss(newline);
			double nextnum;
			nums.resize(0); // dispose of elements (without resetting capacity)
			while(ss >> nextnum) nums.push_back(nextnum);
			
			// clear previous chart so we replace it
			std::clock_t clear_start = std::clock();
			if(!first){
				for (int i= 0; i<=height; i++){
					std::cout << "\033[A\033[2K";
				}
			}
			clock_t clear_end = std::clock();
			first=false;
			
			// scale data to fit window
			if(nums.size()>width){
				int windowsize=ceil(nums.size()/width);
				nums2.resize(0);
				for(int i=0; i<floor(nums.size()/windowsize); ++i){
					int lower=i*windowsize;
					int upper=std::min(nums.size(),size_t((i+1)*windowsize))-1;
					nums2.push_back(std::accumulate(nums.begin()+lower,nums.begin()+upper,0)/windowsize);
				}
				std::swap(nums,nums2);
			}
			
			
			// yeah we just make a new Asciichart every time. :/
			ascii::Asciichart asciichart(nums);
			std::clock_t write_start = std::clock();
			std::cout << asciichart.type(ascii::Asciichart::LINE)
			                       .height(height)
			                       .Plot();
			std::clock_t write_end = std::clock();
			
			//printf("clear took %f, write took %f, total %f\n",
			//       (clear_end-clear_start)*1./CLOCKS_PER_SEC,
			//       (write_end-write_start)*1./CLOCKS_PER_SEC,
			//       (write_end-clear_start)*1./CLOCKS_PER_SEC);
		}
		
		std::cout<<"FIFO closed, re-opening in 5s..."<<std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		first=true;
	}
	
	
	return 0;
}

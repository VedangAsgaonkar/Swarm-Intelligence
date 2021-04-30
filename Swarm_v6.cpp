/*Log: Vedang 28-04-2021 10:48
Implemented random moving ants that can mark path after goal is reached. Will implement Gaussian variastion of angle to avoid too 
much randomness and speed up convergence, and reduce spread of pheromone trail.
*/
/*Log: Vedang 28-04-2021 11:15
To be improved: Bias initial randomness, implement in terms of turning angle, with fixed step size, and random angle being gaussian
centered at 0 radians.
*/
/*Log: Vedang 28-04-2021 11:42
Implements the previous improvement, but seed needs to be randomized
*/
/*Log: Vedang 28-04-2021 12:19
Seed randomization with time is not working, need to figure out the flaw
*/
/*Log: Vedang 28-04-2021 19:35
A pheromone array has been created. On finding the target, all ants re-initialize. Yet to implement pheromone bias.
*/
/*Log: Vedang 28-04-2021 23:05
Pheromone bias has been coded, but quality not good, lot of flaws, need to improve the algorithm, get good hyperparameters.
*/
/*Log: Vedang 29-04-2021 08:34
Restoring v1 to the previous working version, will commit changes to v2.
*/
/*Log: Vedang 29-04-2021 10:47
The v2 version is failing miserably, will create a v3 and try to remake the pheromone code, keeping v2 as it is.
*/
/*Log Vedang 29-04-2021 12:20
The v3 version has changed pheromone amounts to double values. It also gives higher pheromone values closer to target and for 
shorter paths. Two procedures of biasing were implemented. The first is based on vector addition. However it leads to getting
stuck at places where pheromone gradient is small. The second approach finds direction of max pheromone and goes biases in that
direction. It is quite successful. However the ants have become too disciplined and will tend to get stuck at local minima. 
The ways to counter this are:
1) Increase standard deviation
2) Reduce hyperparams::weight
3) Implement segment of disc maximum based biasing
4) Implement a greater number of random exploration cycles, before setting the model to this disciplined state and then use the 
   disciplined state to fine tune
5) Implement pheromone evaporation.
Future versions also need to implement the fact that all ants are not supposed to get recalled when one reaches the target. Create
a new generation of ants. Also get rid of ants that reach the target. The random seed problem also needs to be resolved. Future 
versions may also modify standard deviations based on pheromone concentrations, this can help with the initial over-discipline 
problem.
*/
/*Log: Vedang 29-04-2021 14:30
Changed the standard deviation to PI/8, seems to work better.
*/
/*Log: Vedang 29-04-2021 14:42
Implemented an exponential evaporating pheromone. Outputs vary massively with decay parameter.
*/
/*Log: Vedang 29-04-2021 14:49
Implemented a relu evaporating pheromone. Output is satisfactory for decay = 0.05 and stddev=PI/8 , but shows local accumulation
towards end part. Need to implement an algorithm to prevent local accumulation.
*/
/*Log: Vedang 29-04-2021 18:19
Attempting to reduce local accumulation by giving random breakouts(i.e. no bias)/ random large steps
*/
/*Log: Vedang 29-04-2021 18:25
Experimented with increasing hyperparams::step_size to 35 with hyperparams::ph_rad as 25. Increases indiscipline, but reduces local accumulation. To curb
discipline loss, may implement pheromone distance based bias also, so that parts between hyperparams::ph_rad and 2*hyperparams::ph_rad also have slight 
bias to pheromone
*/
/*Log: Vedang 29-04-2021 18:19
Again raduced hyperparams::step_size to 25. Have not implemented above idea. Implemented random breakouts with a probability 0.01. Quite 
successful in curbing local accumulation. Need to analyze why accumulation occurs only at beginning and finish parts of trail.
May reduce these by doing something different at beginning and end.
*/
/*Log: Vedang 29-04-2021 18:56
Increased hyperparams::step_size to 30, hyperparams::ph_rad is 25, and implemented random breakouts. Works very well.
*/
/*Log: Vedang 29-04-2021 19:41
Packaged hyperparams, organised code.
*/
/*Log: Vedang 29-04-2021 19:51
Tested with different target locations. Works well. Self-corrects. However after too much time, a drift to the left of target is observed.
Also after too much time, quality(number of ants going) reduces. 
Will implement high pheromone density at target.
*/
/*Log: Vedang 29-04-2021 21:38
Implemented high pheromone density at target. Works very well to reduce local accumulation and gives nearly straight line paths.
Similar change in pheromone density can be used to create obstacles in the path. Need to unify target parameters. 
*/
/*Log: Vedang 30-04-2021 10:17
Will attempt multiple ant generations in v6.
*/
/*Log: Vedang 30-04-2021 10:50
Implemented generations. 50 ants in subsequent generations works well. With 10 ants, somehow segmentation fault is occuring. Will try to
implement boundary conditions.
*/
/*Log: Vedang 30-04-2021 11:53
Implemented reflection. Leads to slight quality reduction due to reflected paths, but fair enough. Also gets rid of the segmentation
fault. Generations of 50 seem to give more quality than of 10, but 10 gives narrower paths.
Local accumulation at some places still observed, particularly in the 10 case, 4 local accumulations, two the the left of target, one
at the beginning and one at the left of the major path. Have to figure out why everything is to the left.
*/

//@author: Vedang Asgaonkar


#include<iostream>
#include<cmath>
#include<random>
#include<FL/Fl.H>
#include<FL/Fl_Window.H>
#include<FL/Fl_Box.H>
#include<FL/fl_draw.H>
#include<FL/Fl_Timer.H>
#include<FL/Fl_Button.H>
#include<bits/stdc++.h>

using namespace std ;

const double PI = 3.14;

namespace hyperparams{
	int step_size = 30 ;
	int ph_rad = 25 ;
	double weight = 0.25 ;
	int recipocal_breakout_prob = 100;
	int evap_choice = 1 ; // 0 for exponential, 1 for relu
	double decay = 0.05;
	int num_ants = 100; //needs to be manually set everywhere as variable length array forbidden
	double std_dev = PI/8 ;
}



unsigned int seed;
std::default_random_engine generator(seed);
std::normal_distribution<double> distribution1(0,hyperparams::std_dev);
std::uniform_real_distribution<double> distribution2(0.0,2*PI);
bool run = false ;
bool recall = false ;
int ant_index = 0;


class Pheromone{
	private:
	int tar_x1, tar_x2, tar_y1, tar_y2 ;
	public:
	double ph_arr[1000][1000];
	Pheromone(int tar_x1, int tar_x2, int tar_y1, int tar_y2){
		for(int i=0 ; i<1000 ; i++){
			for(int j=0; j<1000 ; j++){
				if( i<= tar_x2 && i>= tar_x1 && j<=tar_y2 && j>=tar_y1 ){
					ph_arr[i][j]= 200.0;	
				}
				else{
					ph_arr[i][j] = 0.0 ;
				}
			}
		}
	}
	void exponential_evaporate( double decay ){
		for(int i = 0 ; i <1000 ; i++){
			for(int j=0 ; j<1000 ; j++){
				if( i<= tar_x2 && i>= tar_x1 && j<=tar_y2 && j>=tar_y1 ){
							
				}
				else{
					ph_arr[i][j] = ph_arr[i][j]*decay ;
				}
				
			}
		}
	}
	void relu_evaporate( double decay ){
		for(int i = 0 ; i <1000 ; i++){
			for(int j=0 ; j<1000 ; j++){
				if( i<= tar_x2 && i>= tar_x1 && j<=tar_y2 && j>=tar_y1 ){
							
				}
				else{
					if( ph_arr[i][j] > decay ){
					ph_arr[i][j] -= decay ;
				}
				}
				
			}
		}
	}
};

Pheromone ph(190 ,240 , 100 , 150);	


class Shape : public Fl_Widget  {
  
	
	public:
		int x, y, w, h;
		Shape (int x, int y, int w, int h) : Fl_Widget (x,y,w,h){
			this->x=x; this->y=y;
			this->w=w; this->h=h;
		}
		virtual void wipe()=0;
		virtual void refresh(double dx, double dy)=0;
			
		
};

class MyRectangle : public Shape {
	protected:
	int c ;
	public:
	MyRectangle( int x , int y , int w , int h , int cl) : Shape( x , y , w , h ){
		c = cl ;	
	}
	virtual void draw(){
		fl_draw_box(FL_FLAT_BOX , x , y , w , h ,c );
	}
	virtual void wipe(){
		fl_draw_box (FL_FLAT_BOX,x,y,w,h,FL_GRAY);
	}
	virtual void refresh(double dx , double dy){
		wipe();
		x=x+dx;y=y+dy;
		resize(x,y,w,h);
		fl_draw_box (FL_FLAT_BOX,x,y,w,h,c);
	}
	virtual void moveTo(int x1 , int y1 ){
	    wipe();
		x=x1;y=y1;
		resize(x,y,w,h);
		fl_draw_box (FL_FLAT_BOX,x,y,w,h,c);
   };
	
};


class Ant : public MyRectangle{
	private:
		int x1,x2,y1,y2 ;
		int pos[1000][2] ;
		int cnt ;
		
	public:
	bool isDormant ;
	double currAngle ; 
	Ant( int x , int y , int w , int h , int cl , int x1_t , int x2_t , int y1_t , int y2_t) : MyRectangle(x,y,h,w,cl){
		x1 = x1_t ;
		x2 = x2_t ;
		y1 = y1_t ;
		y2 = y2_t ;
		pos[0][0] = x ;
		pos[0][1] = y ;
		cnt = 0 ;
	}
	virtual void refresh(double dx , double dy){
		if(!isDormant){
			wipe();
			x=x+dx;y=y+dy;
			resize(x,y,w,h);
			fl_draw_box (FL_FLAT_BOX,x,y,w,h,c);
			cnt++ ;
			if( cnt < 1000){
				pos[cnt][0] = x ;
				pos[cnt][1] = y ;
			}
			currAngle = atan2( dy , dx );
			if( x1 <= x && x <= x2 && y1 <= y && y <= y2 ){
				for(int i = 0 ; i< cnt ; i++){
					fl_draw_box( FL_FLAT_BOX, pos[i][0] , pos[i][1] , 3 ,3 ,FL_BLACK );
					//run = false ;
					(ph.ph_arr[pos[i][0]][pos[i][1]]) += (i+1)*100.0/cnt ;	
					
				}
				recall = true ;
				isDormant = true ;
			}
		}
	}
	void reinit(){
		cnt=0 ;
	}
	
};


class Wrapper{
	private:
	Ant* arr[1000];
	public:
	Wrapper(Ant* arr1[1000]){
		for(int i = 0 ; i < 1000 ; i++){
			arr[i] = arr1[i] ;
		}
	}
	void refreshAll(){
		//srand(time(NULL));
		for(int i = 0 ; i < 1000 ; i++){
			if(arr[i]->isDormant == false){
				double bias_x =0.0 ;
				double bias_y =0.0;
				int x0 = arr[i]->x ;
				int y0 = arr[i]->y ;
				double max_ph = 0;
				bool ref_right=false ;
				bool ref_left=false;
				bool ref_top=false;
				bool ref_bottom=false;
				
				if( x0 > 950 ){
					ref_right = true ;
				}
				else if( x0 < 50){
					ref_left = true;
				}
				
				if( y0 > 950 ){
					ref_bottom = true ;
				}
				else if( y0 < 50){
					ref_top = true ;
				}
				
				if( ref_top || ref_bottom || ref_right || ref_left ){
					if( ref_top ){
						arr[i]->refresh( 0 , hyperparams::step_size );
					}
					else if( ref_bottom){
						arr[i]->refresh( 0 , -hyperparams::step_size );	
					}
					
					if( ref_left ){
						arr[i]->refresh( hyperparams::step_size , 0 );
					}
					else if( ref_right ){
						arr[i]->refresh( -hyperparams::step_size , 0);	
					}
				}
				else{
					for( int j= -hyperparams::ph_rad ; j <= +hyperparams::ph_rad ; j++){
						for( int k = -floor(sqrt(round(hyperparams::ph_rad*hyperparams::ph_rad-j*j))) ; k <= floor(sqrt(round(hyperparams::ph_rad*hyperparams::ph_rad-j*j))) ; k++){
							if( j!=0 || k!=0){
								if( x0+j<950 && x0+j>50 && y0+k<950 && y0+k>50 ){
									/* //ONE WAY OF DOING THINGS
									bias_x += (ph.ph_arr[x0+j][y0+k])*j/sqrt(j*j+k*k) ;
									bias_y += (ph.ph_arr[x0+j][y0+k])*k/sqrt(j*j+k*k) ;
									*/
									//OTHER WAY: should ideally fragment into 20 parts and see which arc has max pheromone
									if( (ph.ph_arr[x0+j][y0+k]) > max_ph ){
										bias_x += (ph.ph_arr[x0+j][y0+k])*j/sqrt(j*j+k*k) ;
										bias_y += (ph.ph_arr[x0+j][y0+k])*k/sqrt(j*j+k*k) ; 
										max_ph = (ph.ph_arr[x0+j][y0+k]) ;		
									}
									
								}
													
							}	
						}			
					}
					
					double bias_mag = sqrt( bias_x*bias_x + bias_y*bias_y );
					double bias_angle;
					if( bias_mag > 1e-2 ){
						bias_angle = atan2( bias_y , bias_x );
					}
					else{
						bias_angle = 0 ;
					}
					
					double shift;
					if( rand()%hyperparams::recipocal_breakout_prob != 12 ){ 
				 		shift = (bias_angle*bias_mag*hyperparams::weight + arr[i]->currAngle )/(bias_mag*hyperparams::weight + 1 );
					}
					else{ //random breakout can be implemented.
						shift = 0;
					}
					
					double center = distribution1(generator) ; 
					
					arr[i]->refresh( hyperparams::step_size*cos(center+shift) , hyperparams::step_size*sin(center+shift));
				}
			}	
		}	
	}
	void recallAll(){
		int init_ant_index = ant_index ;
		if( ant_index < 1000){
			for( ; ant_index - init_ant_index < 10 ; ant_index++){
				if( ant_index < 1000){
					arr[ant_index]->isDormant=false;
				}
				else{
					break;
				}
			}
		}
		recall = false;
	}
};


void callBack( void* target){
	if(run){
		Wrapper* rptr = (Wrapper*)target ;
		if(!recall){		
			rptr->refreshAll();		
		}
		else{
			rptr->recallAll() ;	
		}
		switch(hyperparams::evap_choice){
		case 0: ph.exponential_evaporate(hyperparams::decay);
				break;
		case 1: ph.relu_evaporate(hyperparams::decay);
				break;
		}
		Fl::repeat_timeout(0.3, callBack , target )	;
	}
}


class MyTimer{
	private:
	
	public:
		MyTimer(){
			run = false ;
		}
		void start(void* target){
			run = true ;
			Fl::repeat_timeout(1.0, callBack , target )	;
		}
		void stop(){
			run = false ;
		}

};


class MyButton : public Fl_Button{
	private:
	
	public : 
	MyButton( int x , int y , int h , int w , char* lbl ) : Fl_Button(x,y,h,w,lbl){
		
	}
	virtual int handle(int e){
		if( e == FL_PUSH ){
			run=false;
			for(int i=0 ; i<1000 ; i++){
			
				for(int j=0 ; j < 1000 ; j++){
					if( ph.ph_arr[i][j] >0){
						fl_draw_box (FL_FLAT_BOX, i , j , 5, 5, 140+ 0.5*ph.ph_arr[i][j]); 
					}
				}
			}	
		}
		return 1;	
	}

};


int main(){
	srand(time(NULL));
	seed = rand();
	
	try{
	Fl_Window* window = new Fl_Window(1000,1000,"My Window"  ) ;
	MyRectangle* finish = new MyRectangle( 190,100,50,50,140);
	Ant* arr[1000] ;
	for(int i = 0 ; i < 1000 ; i++ ){
		arr[i] = new Ant( 500 , 500 , 5 , 5 , 120 , 190 ,240 , 100 , 150);
		arr[i]->currAngle = distribution2(generator);
		if(i<100){
			arr[i]->isDormant = false;
		}
		else{
			arr[i]->isDormant = true ;
		}
	}
	ant_index = 100;
	
	Wrapper* wrapper = new Wrapper(arr);
	 // rect = new MyRectangle( 500 , 500 , 5 , 5 , 120 );
	char carr[5] = {'S','t','o','p','\0'} ;
	MyButton* bt = new MyButton( 700 ,700,30,30,carr);
	
	MyTimer timer ;
	timer.start(wrapper);
	window -> end();
	window -> show();
	
	Fl::run();
	}
	catch( ... ){
		cout << "Finished" <<endl ;
	}
}




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

//@author: Vedang Asgaonkar


#include<iostream>
#include<cmath>
#include<FL/Fl.H>
#include<FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Timer.H>
#include <FL/Fl_Button.H>
#include <bits/stdc++.h>
#include<random>

using namespace std ;

/*namespace myNamespace{
std::mt19937 mersenne{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
std::uniform_int_distribution<> move{ -25 , 25 };
}*/

const double PI = 3.14;
unsigned int seed;

std::default_random_engine generator(seed);


std::normal_distribution<double> distribution1(0,PI/10);
std::uniform_real_distribution<double> distribution2(0.0,2*PI);

bool run = false ;
bool recall = false ;
int step_size = 25 ;


class Pheromone{
	public:
	double ph_arr[1000][1000];
	Pheromone(){
		for(int i=0 ; i<1000 ; i++){
			for(int j=0; j<1000 ; j++){
				ph_arr[i][j] = 0.0 ;
			}
		}
	}
};

Pheromone ph;


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
		wipe();
		x=x+dx;y=y+dy;
		resize(x,y,w,h);
		fl_draw_box (FL_FLAT_BOX,x,y,w,h,c);
		cnt++ ;
		pos[cnt][0] = x ;
		pos[cnt][1] = y ;
		currAngle = atan2( dy , dx );
		if( x1 <= x && x <= x2 && y1 <= y && y <= y2 ){
			for(int i = 0 ; i< cnt ; i++){
				fl_draw_box( FL_FLAT_BOX, pos[i][0] , pos[i][1] , 3 ,3 ,FL_BLACK );
				//run = false ;
				(ph.ph_arr[pos[i][0]][pos[i][1]]) += (i+1)*100.0/cnt   ;
				
			}
			recall = true ;
		}
	}
	void reinit(){
		cnt=0 ;
	}
	
};

int ph_rad = 10 ;
int angle_bias = 1 ;



class Wrapper{
	private:
	Ant* arr[100];
	public:
	Wrapper(Ant* arr1[100]){
		for(int i = 0 ; i < 100 ; i++){
			arr[i] = arr1[i] ;
		}
	}
	void refreshAll(){
		//srand(time(NULL));
		for(int i = 0 ; i < 100 ; i++){
			//This part should work don't know why it doesn't
			double bias_x =0 ;
			double bias_y = 0;
			double ph_qty=0 ;
			
			for( int j = -floor(ph_rad) ; j <= floor(ph_rad) ; j++){
				
				for( int k = - floor(sqrt(ph_rad*ph_rad-j*j)) ; k <= floor(sqrt(ph_rad*ph_rad-j*j)) ; k++){
					
					if( j!=0 || k!=0 ){
						
						if( (arr[i]->x+j)>50 && (arr[i]->x+j)<950 && (arr[i]->y+j)>50 && (arr[i]->y+k)<950){
							
							if( (ph.ph_arr[arr[i]->x + j][arr[i]->x + k]) > 0 ){
								
								bias_x += (ph.ph_arr[arr[i]->x + j][arr[i]->x + k])*j/sqrt(j*j+k*k);
								bias_y += (ph.ph_arr[arr[i]->y + j][arr[i]->y + k])*k/sqrt(j*j+k*k);
								ph_qty += (ph.ph_arr[arr[i]->x + j][arr[i]->x + k]) ;
							}
						}
					}
				
				}
				
			}
			
			double weight = sqrt(bias_x*bias_x + bias_y*bias_y );
			double bias ;
			if( abs(weight) > 1e-2 ){
				bias = atan2(bias_y , bias_x );
			}
			else{
				bias = 0;
			}
			double shift = (bias*weight*angle_bias + arr[i]->currAngle)/(weight*angle_bias+1);
			
			
			
			//arr[i]->refresh( step_size*cos(distribution1(generator)+shift) , step_size*sin(distribution1(generator)+shift)); 
			
			//arr[i]->refresh( reduced_step_size*cos(distribution1(generator)+shift) , reduced_step_size*sin(distribution1(generator)+shift));
			arr[i]->refresh(step_size*cos(distribution1(generator)+shift) ,step_size*sin(distribution1(generator)+shift));
		}	
	}
	void recallAll(){
		for(int i = 0 ; i < 100 ; i++){
			arr[i]->moveTo(500,500);
			arr[i]->currAngle = distribution2(generator);
			arr[i]->reinit();
			recall = false;
		}
		
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
	rand();
	seed = rand();
	
	
	try{
	Fl_Window* window = new Fl_Window(1000,1000,"My Window"  ) ;
	MyRectangle* finish = new MyRectangle( 200,200,50,50,140);
	Ant* arr[100] ;
	for(int i = 0 ; i < 100 ; i++ ){
		arr[i] = new Ant( 500 , 500 , 5 , 5 , 120 , 200 ,250 , 200 , 250);
		arr[i]->currAngle = distribution2(generator);
	}
	
	Wrapper* wrapper = new Wrapper(arr);
	 // rect = new MyRectangle( 500 , 500 , 5 , 5 , 120 );
	char carr[3] = {'G','o','\0'} ;
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




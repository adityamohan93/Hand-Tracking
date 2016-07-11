#include ".handGesture.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include<opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>

using namespace cv;
using namespace std;
HandGesture::HandGesture(){
	frameNumber=0;
	nrNoFinger=0;
	fontFace = FONT_HERSHEY_PLAIN;
}

void HandGesture::initVectors(){
	hullI=vector<vector<int> >(contours.size());
	hullP=vector<vector<Point> >(contours.size());
	defects=vector<vector<Vec4i> > (contours.size());	
}

void HandGesture::analyzeContours(){
	bRect_height=bRect.height;
	bRect_width=bRect.width;
}

string HandGesture::bool2string(bool tf){
	if(tf)
		return "true";
	else
		return "false";
}

string HandGesture::intToString(int number){
		stringstream ss;
		ss << number;
		string str = ss.str();
		return str;
}

void HandGesture::printGestureInfo(Mat src){
	int fontFace = FONT_HERSHEY_PLAIN;
	Scalar fColor(245,200,200);
	int xpos=src.cols/1.5;
	int ypos=src.rows/1.6;
	float fontSize=0.7f;
	int lineChange=14;
	string info= "Figure info:";
	putText(src,info,Point(ypos,xpos),fontFace,fontSize,fColor);
	xpos+=lineChange;
	info=string("Number of defects: ") + string(intToString(nrOfDefects)) ;
	putText(src,info,Point(ypos,xpos),fontFace,fontSize  ,fColor);
	xpos+=lineChange;
	info=string("bounding box height, width ") + string(intToString(bRect_height)) + string(" , ") +  string(intToString(bRect_width)) ;
	putText(src,info,Point(ypos,xpos),fontFace,fontSize ,fColor);
	xpos+=lineChange;
	info=string("Is hand: ") + string(bool2string(isHand));
	putText(src,info,Point(ypos,xpos),fontFace,fontSize  ,fColor);
}

bool HandGesture::detectIfHand(){
	analyzeContours();
	double h = bRect_height; 
	double w = bRect_width;
	isHand=true;
	if(fingerTips.size() > 5 ){
		isHand=false;
	}else if(h==0 || w == 0){
		isHand=false;
	}else if(h/w > 4 || w/h >4){
		isHand=false;	
	}else if(bRect.x<20){
		isHand=false;	
	}	
	return isHand;
}

float HandGesture::distanceP2P(Point a, Point b){
	float d= sqrt(fabs( pow(a.x-b.x,2) + pow(a.y-b.y,2) )) ;  
	return d;
}

void HandGesture::removeRedundantFingerTips(){
	vector<Point> newFingers;
	for(int i=0;i<fingerTips.size();i++){
		for(int j=i;j<fingerTips.size();j++){
			if(distanceP2P(fingerTips[i],fingerTips[j])<10 && i!=j){
			}else{
				newFingers.push_back(fingerTips[i]);	
				break;
			}	
		}	
	}
	fingerTips.swap(newFingers);
}

void HandGesture::computeFingerNumber(){
	std::sort(fingerNumbers.begin(), fingerNumbers.end());
	int frequentNr;	
	int thisNumberFreq=1;
	int highestFreq=1;
	frequentNr=fingerNumbers[0];
	for(int i=1;i<fingerNumbers.size(); i++){
		if(fingerNumbers[i-1]!=fingerNumbers[i]){
			if(thisNumberFreq>highestFreq){
				frequentNr=fingerNumbers[i-1];	
				highestFreq=thisNumberFreq;
			}
			thisNumberFreq=0;	
		}
		thisNumberFreq++;	
	}
	if(thisNumberFreq>highestFreq){
		frequentNr=fingerNumbers[fingerNumbers.size()-1];	
	}
	mostFrequentFingerNumber=frequentNr;	
}

void HandGesture::addFingerNumberToVector(){
	int i=fingerTips.size();	
	fingerNumbers.push_back(i);
}

void HandGesture::addNumberToImg(MyImage *m){
	int xPos=10;
	int yPos=10;
	int offset=30;
	float fontSize=1.5f;
	int fontFace = FONT_HERSHEY_PLAIN;
	for(int i=0;i<numbers2Display.size();i++){
		rectangle(m->src,Point(xPos,yPos),Point(xPos+offset,yPos+offset),numberColor, 2);	
		putText(m->src, intToString(numbers2Display[i]),Point(xPos+7,yPos+offset-3),fontFace,fontSize,numberColor);
		xPos+=40;
		if(xPos>(m->src.cols-m->src.cols/3.2)){
			yPos+=40;
			xPos=10;
		}
	}
}

int terminal_cnt=0,vlc_cnt=0,firefox_cnt=0,prevframe=0,prev=0;
void HandGesture::getFingerNumber(MyImage *m){
	removeRedundantFingerTips();
	if(bRect.height > m->src.rows/2 && nrNoFinger>12 && isHand ){
		numberColor=Scalar(0,200,0);
		addFingerNumberToVector();
		if(frameNumber>12){
			nrNoFinger=0;
			frameNumber=0;	
			computeFingerNumber();	
			numbers2Display.push_back(mostFrequentFingerNumber);
			fingerNumbers.clear();
		}else{
			frameNumber++;
		}
	}else{
		nrNoFinger++;
		numberColor=Scalar(200,200,200);
	}
	int x;
	if(numbers2Display.size()==0)
		x=0;
	else
		x=numbers2Display[numbers2Display.size()-1];
	if(x!=prev)//frameNumber-prevframe>=8 && 
	{
		if(x==4 && terminal_cnt<5)
		{
			prev=x;
			prevframe=frameNumber;
			terminal_cnt++;
			int exit_status = system("gnome-terminal");
		}
		else if(x==1 && vlc_cnt<5)
		{
			prev=x;
			prevframe=frameNumber;
			vlc_cnt++;
			int exit_status = system("vlc &");
		}
		else if((x==2 || x==3 )&& firefox_cnt<5)
		{
			prev=x;
			prevframe=frameNumber;
			firefox_cnt++;
			int exit_status = system("firefox &");
		}
	}
	addNumberToImg(m);
}

float HandGesture::getAngle(Point s, Point f, Point e){
	float l1 = distanceP2P(f,s);
	float l2 = distanceP2P(f,e);
	float dot=(s.x-f.x)*(e.x-f.x) + (s.y-f.y)*(e.y-f.y);
	float angle = acos(dot/(l1*l2));
	angle=angle*180/PI;
	return angle;
}

void HandGesture::eleminateDefects(MyImage *m){
	int tolerance =  bRect_height/5;
	float angleTol=95;
	vector<Vec4i> newDefects;
	int startidx, endidx, faridx;
	vector<Vec4i>::iterator d=defects[cIdx].begin();
	while( d!=defects[cIdx].end() ) {
   	    Vec4i& v=(*d);
	    startidx=v[0]; Point ptStart(contours[cIdx][startidx] );
   		endidx=v[1]; Point ptEnd(contours[cIdx][endidx] );
  	    faridx=v[2]; Point ptFar(contours[cIdx][faridx] );
		if(distanceP2P(ptStart, ptFar) > tolerance && distanceP2P(ptEnd, ptFar) > tolerance && getAngle(ptStart, ptFar, ptEnd  ) < angleTol ){
			if( ptEnd.y > (bRect.y + bRect.height -bRect.height/4 ) ){
			}else if( ptStart.y > (bRect.y + bRect.height -bRect.height/4 ) ){
			}else {
				newDefects.push_back(v);		
			}
		}	
		d++;
	}
	nrOfDefects=newDefects.size();
	defects[cIdx].swap(newDefects);
	removeRedundantEndPoints(defects[cIdx], m);
}

void HandGesture::removeRedundantEndPoints(vector<Vec4i> newDefects,MyImage *m){
	Vec4i temp;
	float avgX, avgY;
	float tolerance=bRect_width/6;
	int startidx, endidx, faridx;
	int startidx2, endidx2;
	for(int i=0;i<newDefects.size();i++){
		for(int j=i;j<newDefects.size();j++){
	    	startidx=newDefects[i][0]; Point ptStart(contours[cIdx][startidx] );
	   		endidx=newDefects[i][1]; Point ptEnd(contours[cIdx][endidx] );
	    	startidx2=newDefects[j][0]; Point ptStart2(contours[cIdx][startidx2] );
	   		endidx2=newDefects[j][1]; Point ptEnd2(contours[cIdx][endidx2] );
			if(distanceP2P(ptStart,ptEnd2) < tolerance ){
				contours[cIdx][startidx]=ptEnd2;
				break;
			}if(distanceP2P(ptEnd,ptStart2) < tolerance ){
				contours[cIdx][startidx2]=ptEnd;
			}
		}
	}
}

void HandGesture::checkForOneFinger(MyImage *m){
	int yTol=bRect.height/6;
	Point highestP;
	highestP.y=m->src.rows;
	vector<Point>::iterator d=contours[cIdx].begin();
	while( d!=contours[cIdx].end() ) {
   	    Point v=(*d);
		if(v.y<highestP.y){
			highestP=v;
			cout<<highestP.y<<endl;
		}
		d++;	
	}int n=0;
	d=hullP[cIdx].begin();
	while( d!=hullP[cIdx].end() ) {
   	    Point v=(*d);
		if(v.y<highestP.y+yTol && v.y!=highestP.y && v.x!=highestP.x){
			n++;
		}
		d++;	
	}if(n==0){
		fingerTips.push_back(highestP);
	}
}

void HandGesture::drawFingerTips(MyImage *m){
	Point p;
	circle(m->src,palm_center1,5,Scalar(144,144,255),3);
	circle(m->src,palm_center1,palm_radius,Scalar(144,144,255),2);
	int k=0;
	for(int i=0;i<fingerTips.size();i++){
		p=fingerTips[i];
		putText(m->src,intToString(i),p-Point(0,30),fontFace, 1.2f,Scalar(200,200,200),2);
   		circle( m->src,p,   5, Scalar(100,255,100), 4 );
   	 }
}
pair<Point,double> circleFromPoints(Point p1, Point p2, Point p3)
{
	double offset = pow(p2.x,2) +pow(p2.y,2);
	double bc =   ( pow(p1.x,2) + pow(p1.y,2) - offset )/2.0;
	double cd =   (offset - pow(p3.x, 2) - pow(p3.y, 2))/2.0;
	double det =  (p1.x - p2.x) * (p2.y - p3.y) - (p2.x - p3.x)* (p1.y - p2.y); 
	double TOL = 0.0000001;
	if (abs(det) < TOL) { return make_pair(Point(0,0),0); }

	double idet = 1/det;
	double centerx =  (bc * (p2.y - p3.y) - cd * (p1.y - p2.y)) * idet;
	double centery =  (cd * (p1.x - p2.x) - bc * (p2.x - p3.x)) * idet;
	double radius = sqrt( pow(p2.x - centerx,2) + pow(p2.y-centery,2));

	return make_pair(Point(centerx,centery),radius);
}
double dist(Point x,Point y)
{
	return (x.x-y.x)*(x.x-y.x)+(x.y-y.y)*(x.y-y.y);
}
void HandGesture::getFingerTips(MyImage *m){
	vector<Point> palm_points;
	Point rough_palm_center;
	vector<pair<Point,double> > palm_centers;
	fingerTips.clear();
	int i=0;
	vector<Vec4i>::iterator d=defects[cIdx].begin();
	while( d!=defects[cIdx].end() ) {
   	    Vec4i& v=(*d);
	    int startidx=v[0]; Point ptStart(contours[cIdx][startidx] );
   		int endidx=v[1]; Point ptEnd(contours[cIdx][endidx] );
  	    int faridx=v[2]; Point ptFar(contours[cIdx][faridx] );
  	    
  	  	rough_palm_center+=ptFar+ptStart+ptEnd;
		palm_points.push_back(ptFar);
		palm_points.push_back(ptStart);
		palm_points.push_back(ptEnd);
		if(i==0){
			fingerTips.push_back(ptStart);
			i++;
		}
		fingerTips.push_back(ptEnd);
		d++;
		i++;
   	}
   	rough_palm_center.x/=defects.size()*3;
	rough_palm_center.y/=defects.size()*3;
	vector<pair<double,int> > distvec;
	for(int j=0;j<palm_points.size();j++)
		distvec.push_back(make_pair(dist(rough_palm_center,palm_points[j]),j));
	sort(distvec.begin(),distvec.end());

	pair<Point,double> soln_circle;
	for(int j=0;j+2<distvec.size();j++)
	{
		Point p1=palm_points[distvec[j+0].second];
		Point p2=palm_points[distvec[j+1].second];
		Point p3=palm_points[distvec[j+2].second];
		soln_circle=circleFromPoints(p1,p2,p3);
		if(soln_circle.second!=0)
			break;
	}
	palm_centers.push_back(soln_circle);
	if(palm_centers.size()>10)
		palm_centers.erase(palm_centers.begin());
	
	Point palm_center;
	double radius=0;
	for(int j=0;j<palm_centers.size();j++)
	{
		palm_center+=palm_centers[j].first;
		radius+=palm_centers[j].second;
	}
	palm_center.x/=palm_centers.size();
	palm_center.y/=palm_centers.size();
	radius/=palm_centers.size();
	palm_center1=palm_center;
	if(radius>80)
		radius=80;
	palm_radius=radius;
	palm_center1.y+=140;
	if(fingerTips.size()==0){
		checkForOneFinger(m);
	}
}


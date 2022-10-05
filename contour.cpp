#include <string.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>


namespace contour
{
#define HOLE_BORDER 1
#define OUTER_BORDER 2

using namespace std;

struct MyBorder {
	int seq_num;
	int border_type;
};

struct MyPoint {
	int row;
	int col;

	MyPoint() {
	}

	MyPoint(int r, int c) {
		row = r;
		col = c;
	}

	void setPoint(int r, int c) {
		row = r;
		col = c;
	}

	bool samePoint(MyPoint p) {
		return row == p.row && col == p.col;
	}
};

struct MyPixel {
	unsigned char red;
	unsigned char blue;
	unsigned char green;

	MyPixel(unsigned char r, unsigned char g, unsigned char b) {
		red = r;
		green = g;
		blue = b;
	}
	void setPixel(unsigned char r, unsigned char g, unsigned char b) {
		red = r;
		green = g;
		blue = b;
	}
};

//struct for storing information on the current border, the first child, next sibling, and the parent.
struct MyNode {
	int parent;
	int first_child;
	int next_sibling;
	MyBorder border;
	MyNode(int p, int fc, int ns) {
		parent = p;
		first_child = fc;
		next_sibling = ns;
	}
	void reset() {
		parent = -1;
		first_child = -1;
		next_sibling = -1;
	}
};

//https://github.com/opencv/opencv/blob/17234f82d025e3bbfbf611089637e5aa2038e7b8/modules/imgproc/src/shapedescr.cpp#L308
double contourArea(vector<MyPoint> contour)
{
	/*
	 param:
	 contour: the list of point(only contain one contour)
	*/
	int npoints = contour.size();
	double a00=0;

	// double prev_x = contour[npoints-1].col;
	// double prev_y = contour[npoints-1].row;
	// for(int i=0;i<npoints;i++)
	// {
	// 	double x=contour[i].col;
	// 	double y=contour[i].row;
	// 	a00 += prev_x*y - prev_y*x;
	// 	prev_x = x; prev_y = y;
	// }

	double prev_x = contour[npoints-1].row;//row，col不影响面积计算结果
	double prev_y = contour[npoints-1].col;
	for(int i=0;i<npoints;i++)
	{
		double x=contour[i].row;
		double y=contour[i].col;
		a00 += prev_x*y - prev_y*x;
		prev_x = x; prev_y = y;
	}

	a00 *= 0.5;
	return abs(a00);

}

//Given a vector of vectors of contours, draw out the contour specified by seq_num
//contours[n-2] contains the nth contour since the first contour starts at 2
void drawContour(vector<vector<MyPoint>> contours, vector<vector<MyPixel>> &color, int seq_num, MyPixel pix) {
	int index = seq_num - 2;
	int r, c;
	for (unsigned int i = 0; i < contours[index].size(); i++) {
		r = contours[index][i].row;
		c = contours[index][i].col;
		color[r][c] = pix;
	}
}

//chooses color for a contour based on its seq_num
MyPixel chooseColor(int n) {
	// switch (n%6) {
	// 	case 0:
	// 		return MyPixel(255,0,0);
	// 	case 1:
	// 		return MyPixel(255, 127, 0);
	// 	case 2:
	// 		return MyPixel(255, 255, 0);
	// 	case 3:
	// 		return MyPixel(0, 255, 0);
	// 	case 4:
	// 		return MyPixel(0, 0, 255);
	// 	case 5:
	// 		return MyPixel(139, 0, 255);
	// }
	return MyPixel(255,0,0);
}

//creates a 2D array of struct MyPixel, which is the 3 channel image needed to convert the 2D vector contours to a drawn bmp file
//uses DFS to step through the hierarchy tree, can be set to draw only the top 2 levels of contours, for example.
vector<vector<MyPixel>> createChannels(int h, int w, vector<MyNode> hierarchy, vector<vector<MyPoint>> contours) {
	queue<int> myQueue;
	vector<vector<MyPixel>> color(h,vector<MyPixel> (w, MyPixel((unsigned char) 0, (unsigned char)0, (unsigned char)0)));
	int seq_num;
	for (int n = hierarchy[0].first_child; n != -1; n = hierarchy[n - 1].next_sibling) {
		myQueue.push(n);
	}
	vector<double> tmp;
	while (!myQueue.empty()) {
		seq_num = myQueue.front();
		drawContour(contours, color, seq_num, chooseColor(seq_num));
		myQueue.pop();
		for (int n = hierarchy[seq_num - 1].first_child; n != -1; n = hierarchy[n - 1].next_sibling) {
			myQueue.push(n);
		}
	}
	return color;
}

//step around a MyPixel CCW
void stepCCW(MyPoint &current, MyPoint pivot) {
	if (current.col > pivot.col)
		current.setPoint(pivot.row - 1, pivot.col);
	else if (current.col < pivot.col)
		current.setPoint(pivot.row + 1, pivot.col);
	else if (current.row > pivot.row)
		current.setPoint(pivot.row, pivot.col + 1);
	else if (current.row < pivot.row)
		current.setPoint(pivot.row, pivot.col - 1);
}

//step around a MyPixel CW
void stepCW(MyPoint &current, MyPoint pivot) {
	if (current.col > pivot.col)
		current.setPoint(pivot.row + 1, pivot.col);
	else if (current.col < pivot.col)
		current.setPoint(pivot.row - 1, pivot.col);
	else if (current.row > pivot.row)
		current.setPoint(pivot.row, pivot.col - 1);
	else if (current.row < pivot.row)
		current.setPoint(pivot.row, pivot.col + 1);
}


//checks if a given MyPixel is out of bounds of the image
bool pixelOutOfBounds(MyPoint p, int numrows, int numcols) {
	return (p.col >= numcols || p.row >= numrows || p.col < 0 || p.row < 0);
}

//marks a MyPixel as examined after passing through
void markExamined(MyPoint mark, MyPoint center, bool checked[4]) {
	//p3.row, p3.col + 1
	int loc = -1;
	//    3
	//  2 x 0
	//    1
	if (mark.col > center.col)
		loc = 0;
	else if (mark.col < center.col)
		loc = 2;
	else if (mark.row > center.row)
		loc = 1;
	else if (mark.row < center.row)
		loc = 3;

	if (loc == -1)
		throw exception("Error: markExamined Failed");

	checked[loc] = true;
	return;
}

//checks if given MyPixel has already been examined
bool isExamined(bool checked[4]) {
	//p3.row, p3.col + 1
	return checked[0];
}


//follows a border from start to finish given a starting MyPoint
void followBorder(vector<vector<int>> &image, int row, int col, MyPoint p2, MyBorder NBD, vector<vector<MyPoint>> &contours) {
	int numrows = image.size();
	int numcols = image[0].size();
	MyPoint current(p2.row, p2.col);
	MyPoint start(row, col);
	vector<MyPoint> point_storage;

	//(3.1)
	//Starting from (i2, j2), look around clockwise the pixels in the neighborhood of (i, j) and find a nonzero MyPixel.
	//Let (i1, j1) be the first found nonzero MyPixel. If no nonzero MyPixel is found, assign -NBD to fij and go to (4).
	do {
		stepCW(current, start);
		if (current.samePoint(p2)) {
			image[start.row][start.col] = -NBD.seq_num;
			point_storage.push_back(start);
			contours.push_back(point_storage);
			return;
		}
	} while (pixelOutOfBounds(current, numrows, numcols) || image[current.row][current.col] == 0);
	MyPoint p1 = current;
	
	//(3.2)
	//(i2, j2) <- (i1, j1) and (i3, j3) <- (i, j).
	
	MyPoint p3 = start;
	MyPoint p4;
	p2 = p1;
	bool checked[4];
//	bool checked[8];
	while (true) {
		//(3.3)
		//Starting from the next element of the MyPixel(i2, j2) in the counterclockwise order, examine counterclockwise the pixels in the
		//neighborhood of the current MyPixel(i3, j3) to find a nonzero MyPixel and let the first one be(i4, j4).
		current = p2;

		for (int i = 0; i < 4; i++)
			checked[i] = false;

		do {
			markExamined(current, p3, checked);
			stepCCW(current, p3);
		} while (pixelOutOfBounds(current, numrows, numcols) || image[current.row][current.col] == 0);
		p4 = current;

		//Change the value fi3, j3 of the MyPixel(i3, j3) as follows :
		//	If the MyPixel(i3, j3 + 1) is a 0 - MyPixel examined in the substep(3.3) then fi3, j3 <- - NBD.
		//	If the MyPixel(i3, j3 + 1) is not a 0 - MyPixel examined in the substep(3.3) and fi3, j3 = 1, then fi3, j3 ←NBD.
		//	Otherwise, do not change fi3, j3.

		if ( (p3.col+1 >= numcols || image[p3.row][p3.col + 1] == 0) && isExamined(checked)) {
			image[p3.row][p3.col] = -NBD.seq_num;
		}
		else if (p3.col + 1 < numcols && image[p3.row][p3.col] == 1) {
			image[p3.row][p3.col] = NBD.seq_num;
		}

		point_storage.push_back(p3);
		//printImage(image, image.size(), image[0].size());
		//(3.5)
		//If(i4, j4) = (i, j) and (i3, j3) = (i1, j1) (coming back to the starting MyPoint), then go to(4);
		//otherwise, (i2, j2) <- (i3, j3), (i3, j3) <- (i4, j4), and go back to(3.3).
		if (p4.samePoint(start) && p3.samePoint(p1)) {
			contours.push_back(point_storage);
			return;
		}

		p2 = p3;
		p3 = p4;
	}
}

//prints the hierarchy list
void printHierarchy(vector<MyNode> hierarchy) {
	for (unsigned int i = 0; i < hierarchy.size(); i++) {
		cout << setw(2) << i + 1 << ":: parent: " <<setw(3)<< hierarchy[i].parent << " first child: " << setw(3) << hierarchy[i].first_child << " next sibling: " << setw(3) << hierarchy[i].next_sibling << endl;
	}
}

void find_contours(vector<vector<int>> image,vector<vector<MyPoint>> &contours,vector<MyNode> &hierarchy)
{
    /*
    param:
     contours:
     a vector of vectors to store each contour.
	 contour n will be stored in contours[n-2]
	 contour 2 will be stored in contours[0], contour 3 will be stored in contours[1], ad infinitum

     hierarchy:
     hierarchy tree will be stored as an vector of nodes instead of using an actual tree since we need to access a MyNode based on its index
	 see definition for MyNode
	 -1 denotes NULL


    */
    int numrows = image.size();
	int numcols = image[0].size();
	MyBorder NBD, LNBD;


	if (image.empty()) {
		throw exception("Image Error");
	}

	LNBD.border_type = HOLE_BORDER;
	NBD.border_type = HOLE_BORDER;
	NBD.seq_num = 1;

	MyNode temp_node(-1, -1, -1);
	temp_node.border = NBD;
	hierarchy.push_back(temp_node);

	MyPoint p2;
	bool border_start_found;
	for (int r = 0; r < numrows; r++) {
		LNBD.seq_num = 1;
		LNBD.border_type = HOLE_BORDER;
		for (int c = 0; c < numcols; c++) {
			border_start_found = false;
			//Phase 1: Find border
			//If fij = 1 and fi, j-1 = 0, then decide that the MyPixel (i, j) is the border following starting MyPoint
			//of an outer border, increment NBD, and (i2, j2) <- (i, j - 1).
			if ((image[r][c] == 1 && c - 1 < 0) || (image[r][c] == 1 && image[r][c - 1] == 0)) {
				NBD.border_type = OUTER_BORDER;
				NBD.seq_num += 1;
				p2.setPoint(r,c-1);
				border_start_found = true;
			}

			//Else if fij >= 1 and fi,j+1 = 0, then decide that the MyPixel (i, j) is the border following
			//starting MyPoint of a hole border, increment NBD, (i2, j2) ←(i, j + 1), and LNBD ← fij in case fij > 1.
			else if ( c+1 < numcols && (image[r][c] >= 1 && image[r][c + 1] == 0)) {
				NBD.border_type = HOLE_BORDER;
				NBD.seq_num += 1;
				if (image[r][c] > 1) {
					LNBD.seq_num = image[r][c];
					LNBD.border_type = hierarchy[LNBD.seq_num-1].border.border_type;
				}
				p2.setPoint(r, c + 1);
				border_start_found = true;
			}
			if (border_start_found) {
				//Phase 2: Store Parent

//				current = new TreeNode(NBD);
				temp_node.reset();
				if (NBD.border_type == LNBD.border_type) {
					temp_node.parent = hierarchy[LNBD.seq_num - 1].parent;
					temp_node.next_sibling = hierarchy[temp_node.parent - 1].first_child;
					hierarchy[temp_node.parent - 1].first_child = NBD.seq_num;
					temp_node.border = NBD;
					hierarchy.push_back(temp_node);

					// cout << "indirect: " << NBD.seq_num << "  parent: " << LNBD.seq_num <<endl;
				}
				else {
					if (hierarchy[LNBD.seq_num-1].first_child != -1) {
						temp_node.next_sibling = hierarchy[LNBD.seq_num-1].first_child;
					}

					temp_node.parent = LNBD.seq_num;
					hierarchy[LNBD.seq_num-1].first_child = NBD.seq_num;
					temp_node.border = NBD;
					hierarchy.push_back(temp_node);

					// cout << "direct: " << NBD.seq_num << "  parent: " << LNBD.seq_num << endl;
				}
				//Phase 3: Follow border
				followBorder(image, r, c, p2, NBD, contours);
			}

			//Phase 4: Continue to next border
			//If fij != 1, then LNBD <- abs( fij ) and resume the raster scan from the MyPixel(i, j + 1).
			//The algorithm terminates when the scan reaches the lower right corner of the picture.
			if (abs(image[r][c]) > 1) {
				LNBD.seq_num = abs(image[r][c]);
				LNBD.border_type = hierarchy[LNBD.seq_num - 1].border.border_type;
			}
		}
	}
}



}


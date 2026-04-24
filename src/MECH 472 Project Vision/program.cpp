//STANDARD IMPORTS

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <Windows.h>

using namespace std;

#include "image_transfer.h"
#include "vision.h"

#include "my_functions.h"

void find_robots(int nblobs, double** blob_matrix, double robot_ic[2], double robot_jc[2], double robot_theta[2], bool robot_found[2]);

bool UseCamera = false;


int main()
{
    int nblobs;
    image rgb, rgb0, label; // declare some image structures
    image grey_image;

    activate_vision();

    rgb.type = RGB_IMAGE;    rgb.width = 640;  rgb.height = 480;
    allocate_image(rgb);
    rgb0.type = RGB_IMAGE;    rgb0.width = 640;  rgb0.height = 480;
    allocate_image(rgb0);

    label.type = LABEL_IMAGE;  label.width = 640;  label.height = 480;
    allocate_image(label);
    grey_image.type = GREY_IMAGE;   grey_image.width = 640;  grey_image.height = 480;
    allocate_image(grey_image);

    load_image(rgb, rgb0, UseCamera);
    //rbg0 staores an orignal copy of rgb


    view_rgb_image(rgb);
    cout << "\nPreview loaded image rgb";
    pause();

    nblobs = process_image(rgb, label);



    /////////////////////////////////////////////// Find Robots ///////////////////////////////////////////////////////
    // Allocate memory for blob analysis
    double** blob_matrix = new double* [nblobs];
    for (int i = 0; i < nblobs; i++) {
        blob_matrix[i] = new double[5];
    }


    // Find/Process average color and display centorid of that color
    double R_ave, G_ave, B_ave;
    double ic, jc;
    copy(rgb, grey_image);

    for (int i = 1; i <= nblobs; i++) {
        // use the original colour image rgb0 here
        centroid(grey_image, label, i, ic, jc);
        average_colour(rgb0, label, i, R_ave, G_ave, B_ave);

        cout << "\nBlob #" << i;
        cout << "\nR_ave = " << R_ave;
        cout << "\nG_ave = " << G_ave;
        cout << "\nB_ave = " << B_ave;

        // mark the centroid point on the image with a colored point
        draw_point_rgb(rgb, (int)ic, (int)jc, R_ave, G_ave, B_ave);

        // Store in the matrix (remember matrix index is 0-based, blob loop is 1-based)
        blob_matrix[i - 1][0] = ic;
        blob_matrix[i - 1][1] = jc;
        blob_matrix[i - 1][2] = R_ave;
        blob_matrix[i - 1][3] = G_ave;
        blob_matrix[i - 1][4] = B_ave;
    }

    // Prepare Output arrays
    double robot_centers_i[2];
    double robot_centers_j[2];
    double robot_angles[2];
    bool robot_status[2];

    find_robots(nblobs, blob_matrix, robot_centers_i, robot_centers_j, robot_angles, robot_status);

    if (robot_status[0]) {
        cout << "\nRed-Green Robot: (" << robot_centers_i[0] << ", " << robot_centers_j[0] << ") at " << robot_angles[0] << " deg";
    }
    draw_point_rgb(rgb, (int)robot_centers_i[0], (int)robot_centers_j[0], 255, 182, 193);
    if (robot_status[1]) {
        cout << "\nBlue-Orange Robot: (" << robot_centers_i[1] << ", " << robot_centers_j[1] << ") at " << robot_angles[1] << " deg";
    }
    draw_point_rgb(rgb, (int)robot_centers_i[1], (int)robot_centers_j[1], 255, 182, 193);


    // Must delete the columns first, then the rows!
    for (int i = 0; i < nblobs; i++) {
        delete[] blob_matrix[i];
    }
    delete[] blob_matrix;


    view_rgb_image(rgb);
    cout << "\nAverage color centroids displayed";
    pause();

    /////////////////////////////////////////////// Find Robots ///////////////////////////////////////////////////////






    copy(rgb0, rgb);
    process_image(rgb, label, true, true);

    //Process and display hue_image
    calculate_hue_image(rgb0, grey_image);
    copy(grey_image, rgb);
    view_rgb_image(rgb);
    cout << "\nHue image displayed";
    pause();



    //Process and display Sobel images
    copy(rgb0, grey_image);
    display_sobel_images(grey_image);




    // Always free in the same order you allocated
    free_image(rgb);
    free_image(rgb0);
    free_image(label);
    free_image(grey_image);
    deactivate_vision();
    
    cout << "\n\ndone.\n";
    return 0;
}




// Function to find robots using a 2D dynamic matrix.
// Matrix structure per row: [0] = ic, [1] = jc, [2] = R, [3] = G, [4] = B
void find_robots(int nblobs, double** blob_matrix, double robot_ic[2], double robot_jc[2], double robot_theta[2], bool robot_found[2])
{
    robot_found[0] = false;
    robot_found[1] = false;

    double red_i = -1, red_j = -1;
    double green_i = -1, green_j = -1;
    double blue_i = -1, blue_j = -1;
    double orange_i = -1, orange_j = -1;

    double hue, sat, value;

    // 1. Loop through the rows of the matrix
    for (int k = 0; k < nblobs; k++) {
        // Extract data from the matrix for readability
        double ic = blob_matrix[k][0];
        double jc = blob_matrix[k][1];
        int R = (int)blob_matrix[k][2];
        int G = (int)blob_matrix[k][3];
        int B = (int)blob_matrix[k][4];

        // Convert to HSV
        calculate_HSV(R, G, B, hue, sat, value);

        /*
        Red: hue > 340 OR hue < 20
        Orange: hue > 20 AND hue < 45
        Green: hue > 90 AND hue < 170
        Blue: hue > 200 AND hue < 260
        */

        if (sat > 0.3) { // Discard grey marks
            // Classify by Hue Ranges
            if (hue < 20 || hue > 340) {
                red_i = ic; red_j = jc;
                cout << "\nFound red centroid!";
            }
            else if (hue >= 20 && hue <= 60) {
                orange_i = ic; orange_j = jc;
                cout << "\nFound orange centroid!";
            }
            else if (hue >= 90 && hue <= 170) {
                green_i = ic; green_j = jc;
                cout << "\nFound green centroid!";
            }
            else if (hue >= 200 && hue <= 260) {
                blue_i = ic; blue_j = jc;
                cout << "\nFound blue centroid!";
            }
        }
    }

    // 2. Robot 0 (Red/Green)
    if (red_i != -1 && green_i != -1) {
        robot_found[0] = true;
        robot_ic[0] = (red_i + green_i) / 2.0;
        robot_jc[0] = (red_j + green_j) / 2.0;
        robot_theta[0] = atan2(green_j - red_j, green_i - red_i) * (180.0 / 3.14159);
    }

    // 3. Robot 1 (Blue/Orange)
    if (blue_i != -1 && orange_i != -1) {
        robot_found[1] = true;
        robot_ic[1] = (blue_i + orange_i) / 2.0;
        robot_jc[1] = (blue_j + orange_j) / 2.0;
        robot_theta[1] = atan2(orange_j - blue_j, orange_i - blue_i) * (180.0 / 3.14159);
    }
}
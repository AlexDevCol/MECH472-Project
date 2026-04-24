#pragma once

int load_image(image& rgb, image& rgb0, bool useCamera = false, int cam_number = 0);
int process_image(image& rgb, image& label, bool draw_ics = false, bool light_bg = false);

void calculate_HSV(int R, int G, int B, double& hue, double& sat, double& value);
void calculate_hue_image(image& rgb, image& hue_image);

void display_sobel_images(image& grey_image);
int sobel(image& a, image& mag, image& theta);

void average_colour(image& rgb, image& label_image, int label_num, double& R, double& G, double& B);
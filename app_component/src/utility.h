
#ifndef UTILITY_H
#define UTILITY_H

#include "layer.h"



typedef struct {
    float dx1;
    float dy1;
    float dx2;
    float dy2;
    float q1_x;
    float q1_y;
    float q2_x;
    float q2_y;
    float score;
    float nscore;
} BoundingBox;

// // apply thresholding to the scores
// void threshold_scores(Layer *layer, float threshold, BoundingBox* boxes, int* num_boxes);

// // apply bounding box offsets
// void apply_offsets(Layer *layer, BoundingBox* boxes, int num_boxes);

// // Helper function to calculate Intersection over Union (IoU)
// float calculate_iou(BoundingBox box1, BoundingBox box2);

// // Non-max suppression (NMS) to filter overlapping boxes
// void non_max_suppression(BoundingBox* boxes, int* num_boxes);;

// Utility function to perform NMS
// void non_maximum_suppression(float *boxes, int num_boxes, float threshold, const char *method, int *pick, int *num_pick);

// Function to generate bounding boxes from CNN outputs
void generate_bounding_boxes(Layer * layer_imap, Layer *reg, int width, int height, float scale, float threshold, BoundingBox **boundingbox, int *num_boxes);

void image_resize(float* input, float* output, u32 height, u32 width, float scale_factor);

#endif // !UTILITY_H
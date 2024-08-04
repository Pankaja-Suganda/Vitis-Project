
#ifndef UTILITY_H
#define UTILITY_H

#include "layer.h"

typedef struct {
    float x, y, w, h;
    float score;
} BoundingBox;

// apply thresholding to the scores
void threshold_scores(Layer *layer, float threshold, BoundingBox* boxes, int* num_boxes);

// apply bounding box offsets
void apply_offsets(Layer *layer, BoundingBox* boxes, int num_boxes);

// Helper function to calculate Intersection over Union (IoU)
float calculate_iou(BoundingBox box1, BoundingBox box2);

// Non-max suppression (NMS) to filter overlapping boxes
void non_max_suppression(BoundingBox* boxes, int* num_boxes);;

#endif // !UTILITY_H
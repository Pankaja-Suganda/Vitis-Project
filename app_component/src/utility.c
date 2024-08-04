#include "utility.h"
#include <math.h>

#define STRIDE 2
#define CELLSIZE 12

// Utility function to transpose a matrix
void transpose(float *src, float *dest, int width, int height) {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            dest[j * height + i] = src[i * width + j];
        }
    }
}

// Utility function to perform NMS
void non_maximum_suppression(float *boxes, int num_boxes, float threshold, const char *method, int *pick, int *num_pick) {
    if (num_boxes == 0) {
        *num_pick = 0;
        return;
    }

    float *x1 = (float *)malloc(num_boxes * sizeof(float));
    float *y1 = (float *)malloc(num_boxes * sizeof(float));
    float *x2 = (float *)malloc(num_boxes * sizeof(float));
    float *y2 = (float *)malloc(num_boxes * sizeof(float));
    float *s = (float *)malloc(num_boxes * sizeof(float));
    float *area = (float *)malloc(num_boxes * sizeof(float));

    // Extract box coordinates and scores
    for (int i = 0; i < num_boxes; ++i) {
        x1[i] = boxes[i * 5 + 0];
        y1[i] = boxes[i * 5 + 1];
        x2[i] = boxes[i * 5 + 2];
        y2[i] = boxes[i * 5 + 3];
        s[i] = boxes[i * 5 + 4];
        area[i] = (x2[i] - x1[i] + 1) * (y2[i] - y1[i] + 1);
    }

    int *sorted_indices = (int *)malloc(num_boxes * sizeof(int));
    for (int i = 0; i < num_boxes; ++i) {
        sorted_indices[i] = i;
    }

    // Sort indices by scores
    for (int i = 0; i < num_boxes - 1; ++i) {
        for (int j = i + 1; j < num_boxes; ++j) {
            if (s[sorted_indices[i]] < s[sorted_indices[j]]) {
                int temp = sorted_indices[i];
                sorted_indices[i] = sorted_indices[j];
                sorted_indices[j] = temp;
            }
        }
    }

    int count = 0;
    for (int i = 0; i < num_boxes; ++i) {
        int idx = sorted_indices[i];
        int keep = 1;
        for (int j = 0; j < count; ++j) {
            int other_idx = pick[j];
            float xx1 = fmaxf(x1[idx], x1[other_idx]);
            float yy1 = fmaxf(y1[idx], y1[other_idx]);
            float xx2 = fminf(x2[idx], x2[other_idx]);
            float yy2 = fminf(y2[idx], y2[other_idx]);

            float w = fmaxf(0.0f, xx2 - xx1 + 1);
            float h = fmaxf(0.0f, yy2 - yy1 + 1);
            float inter = w * h;

            float o;
            if (strcmp(method, "Min") == 0) {
                o = inter / fminf(area[idx], area[other_idx]);
            } else {
                o = inter / (area[idx] + area[other_idx] - inter);
            }

            if (o > threshold) {
                keep = 0;
                break;
            }
        }
        if (keep) {
            pick[count++] = idx;
        }
    }

    *num_pick = count;

    free(x1);
    free(y1);
    free(x2);
    free(y2);
    free(s);
    free(area);
    free(sorted_indices);
}

// Function to generate bounding boxes from CNN outputs
void generate_bounding_boxes(float *imap, float *reg, int width, int height, float scale, float threshold, float **boundingbox, int *num_boxes) {
    float *transposed_imap = (float *)malloc(width * height * sizeof(float));
    float *transposed_dx1 = (float *)malloc(width * height * sizeof(float));
    float *transposed_dy1 = (float *)malloc(width * height * sizeof(float));
    float *transposed_dx2 = (float *)malloc(width * height * sizeof(float));
    float *transposed_dy2 = (float *)malloc(width * height * sizeof(float));
    
    transpose(imap, transposed_imap, width, height);
    transpose(reg, transposed_dx1, width, height);
    transpose(reg + width * height, transposed_dy1, width, height);
    transpose(reg + 2 * width * height, transposed_dx2, width, height);
    transpose(reg + 3 * width * height, transposed_dy2, width, height);

    int *temp_boxes = (int *)malloc(width * height * sizeof(int));
    int box_count = 0;
    
    for (int i = 0; i < width * height; ++i) {
        if (transposed_imap[i] >= threshold) {
            temp_boxes[box_count++] = i;
        }
    }
    
    if (box_count == 0) {
        *boundingbox = NULL;
        *num_boxes = 0;
        free(transposed_imap);
        free(transposed_dx1);
        free(transposed_dy1);
        free(transposed_dx2);
        free(transposed_dy2);
        free(temp_boxes);
        return;
    }
    
    *boundingbox = (float *)malloc(box_count * 7 * sizeof(float));
    *num_boxes = box_count;

    for (int i = 0; i < box_count; ++i) {
        int idx = temp_boxes[i];
        int y = idx / width;
        int x = idx % width;

        float score = transposed_imap[idx];
        float dx1 = transposed_dx1[idx];
        float dy1 = transposed_dy1[idx];
        float dx2 = transposed_dx2[idx];
        float dy2 = transposed_dy2[idx];

        float q1_x = roundf((STRIDE * x + 1) / scale);
        float q1_y = roundf((STRIDE * y + 1) / scale);
        float q2_x = roundf((STRIDE * x + CELLSIZE) / scale);
        float q2_y = roundf((STRIDE * y + CELLSIZE) / scale);

        (*boundingbox)[i * 7 + 0] = q1_x;
        (*boundingbox)[i * 7 + 1] = q1_y;
        (*boundingbox)[i * 7 + 2] = q2_x;
        (*boundingbox)[i * 7 + 3] = q2_y;
        (*boundingbox)[i * 7 + 4] = score;
        (*boundingbox)[i * 7 + 5] = dx1;
        (*boundingbox)[i * 7 + 6] = dy1;
        (*boundingbox)[i * 7 + 7] = dx2;
        (*boundingbox)[i * 7 + 8] = dy2;
    }

    free(transposed_imap);
    free(transposed_dx1);
    free(transposed_dy1);
    free(transposed_dx2);
    free(transposed_dy2);
    free(temp_boxes);
}

// void threshold_scores(Layer *layer, float threshold, BoundingBox* boxes, int* num_boxes) {
//     int height = layer->output_channels.channels->data.height;
//     int width  = layer->output_channels.channels->data.width;
//     *num_boxes = 0;

//     Channel_Node *second = layer->output_channels.channels->next;

//     float *chan = (float*)second->data.output_ptr;

//     for (int y = 0; y < height; y++) {
//         for (int x = 0; x < width; x++) {
//             float face_score = chan[(y*height)+x]; 
//             if (face_score > threshold) {
//                 boxes[*num_boxes].x = x;
//                 boxes[*num_boxes].y = y;
//                 boxes[*num_boxes].score = face_score;
//                 (*num_boxes)++;
//             }
//         }
//     }
// }

// void apply_offsets(Layer *layer, BoundingBox* boxes, int num_boxes) {
//     Channel_Node *chan_node_1 = layer->output_channels.channels;
//     Channel_Node *chan_node_2 = chan_node_1->next;
//     Channel_Node *chan_node_3 = chan_node_2->next;
//     Channel_Node *chan_node_4 = chan_node_3->next;

//     int height = chan_node_1->data.height;
//     int width  = chan_node_1->data.width;

//     for (int i = 0; i < num_boxes; i++) {
//         int x = (int)boxes[i].x;
//         int y = (int)boxes[i].y;
//         float dx = *(float*)&chan_node_1->data.output_ptr[(y*height)+x];
//         float dy = *(float*)&chan_node_2->data.output_ptr[(y*height)+x];
//         float dw = *(float*)&chan_node_3->data.output_ptr[(y*height)+x];
//         float dh = *(float*)&chan_node_4->data.output_ptr[(y*height)+x];
//         printf("box %d -  (%d, %d) dx(%f) dy(%f) dw(%f) dh(%f) \n", i, x, y, dx, dy, dw, dh);

//         boxes[i].x = x - dx * width;
//         boxes[i].y = y - dy * height;
//         boxes[i].w = width + dw * width;
//         boxes[i].h = height + dh * height;
//     }
// }

// float calculate_iou(BoundingBox box1, BoundingBox box2) {
//     float x1 = fmax(box1.x, box2.x);
//     float y1 = fmax(box1.y, box2.y);
//     float x2 = fmin(box1.x + box1.w, box2.x + box2.w);
//     float y2 = fmin(box1.y + box1.h, box2.y + box2.h);

//     float inter_area = fmax(0, x2 - x1) * fmax(0, y2 - y1);
//     float box1_area = box1.w * box1.h;
//     float box2_area = box2.w * box2.h;

//     return inter_area / (box1_area + box2_area - inter_area);
// }

// #define NMS_THRESHOLD 0.5

// void non_max_suppression(BoundingBox* boxes, int* num_boxes) {
//     u8* keep = (u8*)malloc(*num_boxes * sizeof(u8));
//     for (int i = 0; i < *num_boxes; i++) {
//         keep[i] = 1;
//     }

//     for (int i = 0; i < *num_boxes; i++) {
//         if (!keep[i]) continue;
//         for (int j = i + 1; j < *num_boxes; j++) {
//             if (keep[j] && calculate_iou(boxes[i], boxes[j]) > NMS_THRESHOLD) {
//                 keep[j] = 0;
//             }
//         }
//     }

//     int k = 0;
//     for (int i = 0; i < *num_boxes; i++) {
//         if (keep[i]) {
//             boxes[k++] = boxes[i];
//         }
//     }
//     *num_boxes = k;

//     free(keep);
// }



// int main() {
//     // Example usage
//     int width = 45, height = 45;
//     float scale = 1.0f;
//     float threshold = 0.5f;
    
//     // Example data
//     float *imap = (float *)malloc(width * height * sizeof(float));
//     float *reg = (float *)malloc(width * height * 4 * sizeof(float));
    
//     // Populate `imap` and `reg` with data

//     float *boundingbox;
//     int num_boxes;
    
//     generate_bounding_boxes(imap, reg, width, height, scale, threshold, &boundingbox, &num_boxes);
    
//     if (num_boxes > 0) {
//         int *pick = (int *)malloc(num_boxes * sizeof(int));
//         int num_pick;
        
//         non_maximum_suppression(boundingbox, num_boxes, 0.7f, "Union", pick, &num_pick);

//         // Print bounding boxes
//         for (int i = 0; i < num_pick; ++i) {
//             int idx = pick[i];
//             printf("Box %d: [%.2f, %.2f, %.2f, %.2f] Score: %.2f\n", i,
//                    boundingbox[idx * 7 + 0], boundingbox[idx * 7 + 1],
//                    boundingbox[idx * 7 + 2], boundingbox[idx * 7 + 3],
//                    boundingbox[idx * 7 + 4]);
//         }
        
//         free(pick);
//     }

//     free(imap);
//     free(reg);
//     free(boundingbox);

//     return 0;
// }
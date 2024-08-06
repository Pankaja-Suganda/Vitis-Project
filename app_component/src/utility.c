#include "utility.h"
#include <math.h>
#include <stdio.h>

#define STRIDE 2
#define CELLSIZE 12
#define NMS_THRESHOLD 0.3

// // Utility function to transpose a matrix
// void transpose(float *src, float *dest, int width, int height) {
//     for (int i = 0; i < height; ++i) {
//         for (int j = 0; j < width; ++j) {
//             dest[j * height + i] = src[i * width + j];
//         }
//     }
// }

// // Utility function to perform NMS
// void non_maximum_suppression(float *boxes, int num_boxes, float threshold, const char *method, int *pick, int *num_pick) {
//     if (num_boxes == 0) {
//         *num_pick = 0;
//         return;
//     }

//     float *x1 = (float *)malloc(num_boxes * sizeof(float));
//     float *y1 = (float *)malloc(num_boxes * sizeof(float));
//     float *x2 = (float *)malloc(num_boxes * sizeof(float));
//     float *y2 = (float *)malloc(num_boxes * sizeof(float));
//     float *s = (float *)malloc(num_boxes * sizeof(float));
//     float *area = (float *)malloc(num_boxes * sizeof(float));

//     // Extract box coordinates and scores
//     for (int i = 0; i < num_boxes; ++i) {
//         x1[i] = boxes[i * 5 + 0];
//         y1[i] = boxes[i * 5 + 1];
//         x2[i] = boxes[i * 5 + 2];
//         y2[i] = boxes[i * 5 + 3];
//         s[i] = boxes[i * 5 + 4];
//         area[i] = (x2[i] - x1[i] + 1) * (y2[i] - y1[i] + 1);
//     }

//     int *sorted_indices = (int *)malloc(num_boxes * sizeof(int));
//     for (int i = 0; i < num_boxes; ++i) {
//         sorted_indices[i] = i;
//     }

//     // Sort indices by scores
//     for (int i = 0; i < num_boxes - 1; ++i) {
//         for (int j = i + 1; j < num_boxes; ++j) {
//             if (s[sorted_indices[i]] < s[sorted_indices[j]]) {
//                 int temp = sorted_indices[i];
//                 sorted_indices[i] = sorted_indices[j];
//                 sorted_indices[j] = temp;
//             }
//         }
//     }

//     int count = 0;
//     for (int i = 0; i < num_boxes; ++i) {
//         int idx = sorted_indices[i];
//         int keep = 1;
//         for (int j = 0; j < count; ++j) {
//             int other_idx = pick[j];
//             float xx1 = fmaxf(x1[idx], x1[other_idx]);
//             float yy1 = fmaxf(y1[idx], y1[other_idx]);
//             float xx2 = fminf(x2[idx], x2[other_idx]);
//             float yy2 = fminf(y2[idx], y2[other_idx]);

//             float w = fmaxf(0.0f, xx2 - xx1 + 1);
//             float h = fmaxf(0.0f, yy2 - yy1 + 1);
//             float inter = w * h;

//             float o;
//             if (strcmp(method, "Min") == 0) {
//                 o = inter / fminf(area[idx], area[other_idx]);
//             } else {
//                 o = inter / (area[idx] + area[other_idx] - inter);
//             }

//             if (o > threshold) {
//                 keep = 0;
//                 break;
//             }
//         }
//         if (keep) {
//             pick[count++] = idx;
//         }
//     }

//     *num_pick = count;

//     free(x1);
//     free(y1);
//     free(x2);
//     free(y2);
//     free(s);
//     free(area);
//     free(sorted_indices);
// }

// Function to generate bounding boxes from CNN outputs
void generate_bounding_boxes(Layer * layer_imap, Layer *reg, int width, int height, float scale, float threshold, BoundingBox **boundingbox, int *num_boxes) {
    Channel_Node *chan_node_1 = (Channel_Node *)reg->output_channels.channels;
    Channel_Node *chan_node_2 = (Channel_Node *)chan_node_1->next;
    Channel_Node *chan_node_3 = (Channel_Node *)chan_node_2->next;
    Channel_Node *chan_node_4 = (Channel_Node *)chan_node_3->next;

    Channel_Node *no_face_score = (Channel_Node *)layer_imap->output_channels.channels;
    Channel_Node *face_score    = (Channel_Node *)no_face_score->next;

    width = chan_node_1->data.width;
    height = chan_node_1->data.height;

    // for(int h = 0; h < height; h++){
    //     for(int w = 0; w < width; w++){
    //         if(*(float*)&no_face_score->data.output_ptr[(h*height)+w] != 1.0){
    //             printf("I(%d) H(%d) W(%d) NF(%f) F(%f) \n", (h*height)+w, h, w, 
    //             *(float*)&no_face_score->data.output_ptr[(h*height)+w],
    //             *(float*)&face_score->data.output_ptr[(h*height)+w]);
    //         }

    //     }
    // }
    float *transposed_nimap = (float *)no_face_score->data.output_ptr;
    float *transposed_imap = (float *)face_score->data.output_ptr;
    float *transposed_dx1 = (float *)chan_node_1->data.output_ptr;
    float *transposed_dy1 = (float *)chan_node_2->data.output_ptr;
    float *transposed_dx2 = (float *)chan_node_3->data.output_ptr;
    float *transposed_dy2 = (float *)chan_node_4->data.output_ptr;

    int *temp_boxes = (int *)malloc(width * height * sizeof(int));
    int box_count = 0;
    
    for (int i = 0; i < width * height; ++i) {
        if (transposed_imap[i] >= threshold && transposed_nimap[i] <= 0.5 ) {
            temp_boxes[box_count++] = i;
        }
    }
    
    // printf("Detected Box Count %d \n", box_count);
    if (box_count == 0) {
        *boundingbox = NULL;
        *num_boxes = 0;
    }
    
    *boundingbox = (float *)malloc(box_count * sizeof(BoundingBox));
    *num_boxes = box_count;

    for (int i = 0; i < box_count; ++i) {
        int idx = temp_boxes[i];
        int y = idx / width;
        int x = idx % width;

        float score = transposed_imap[idx];
        float nscore = transposed_nimap[idx];
        float dx1 = transposed_dx1[idx];
        float dy1 = transposed_dy1[idx];
        float dx2 = transposed_dx2[idx];
        float dy2 = transposed_dy2[idx];

        float q1_x = roundf((STRIDE * x + 1) / scale);
        float q1_y = roundf((STRIDE * y + 1) / scale);
        float q2_x = roundf((STRIDE * x + CELLSIZE) / scale);
        float q2_y = roundf((STRIDE * y + CELLSIZE) / scale);

        (*boundingbox)[i].q1_x  = q1_x;
        (*boundingbox)[i].q1_y  = q1_y;
        (*boundingbox)[i].q2_x  = q2_x;
        (*boundingbox)[i].q2_y  = q2_y;
        (*boundingbox)[i].score = score;
        (*boundingbox)[i].nscore = nscore;
        (*boundingbox)[i].dx1   = dx1;
        (*boundingbox)[i].dy1   = dy1;
        (*boundingbox)[i].dx2   = dx2;
        (*boundingbox)[i].dy2   = dy2;

        printf("I(%d) score(%f) nscore(%f) q1_x(%f) q1_y(%f) q2_x(%f) q2_y(%f) dx1(%f) dy1(%f) dx2(%f) dy2(%f)\n",
            idx, score, nscore, q1_x + dx1, q1_y + dy1, q2_x + dx2, q2_y + dy2, dx1, dy1, dx2, dy2
        );

    }

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

float calculate_iou(BoundingBox box1, BoundingBox box2) {
    float x1 = fmax(box1.q1_x, box2.q1_x);
    float y1 = fmax(box1.q1_y, box2.q1_y);
    float x2 = fmin(box1.q2_x, box2.q2_x);
    float y2 = fmin(box1.q2_y, box2.q2_y);

    float inter_area = fmax(0, x2 - x1) * fmax(0, y2 - y1);
    float box1_area = (box1.q2_x - box1.q1_x) * (box1.q2_y - box1.q1_y);
    float box2_area = (box2.q2_x - box2.q1_x) * (box2.q2_y - box2.q1_y);

    // printf("inter area %f : (%f, %f) - (%f, %f) \n", (inter_area / (box1_area + box2_area - inter_area)), 
    //     box1.q1_x, box1.q1_y, box2.q1_x, box2.q1_y);
    return inter_area / (box1_area + box2_area - inter_area);
}

// #define NMS_THRESHOLD 0.5

void non_max_suppression(BoundingBox* boxes, int* num_boxes) {
    u8* keep = (u8*)malloc(*num_boxes * sizeof(u8));
    for (int i = 0; i < *num_boxes; i++) {
        keep[i] = 1;
    }

    for (int i = 0; i < *num_boxes; i++) {
        if (!keep[i]) continue;
        for (int j = i + 1; j < *num_boxes; j++) {
            if (keep[j] && calculate_iou(boxes[i], boxes[j]) > NMS_THRESHOLD) {
                keep[j] = 0;
            }
        }
    }

    int k = 0;
    for (int i = 0; i < *num_boxes; i++) {
        if (keep[i]) {
            boxes[k++] = boxes[i];
        }
    }
    *num_boxes = k;

    free(keep);
}

void rerec(BoundingBox* boxes, int num_boxes) {
    for (int i = 0; i < num_boxes; ++i) {
        float w = boxes[i].q2_x - boxes[i].q1_x;
        float h = boxes[i].q2_y - boxes[i].q1_y;

        if (w < h) {
            float offset = (h - w) * 0.5f;
            boxes[i].q1_x -= offset;
            boxes[i].q2_x += offset;
        } else {
            float offset = (w - h) * 0.5f;
            boxes[i].q1_y -= offset;
            boxes[i].q2_y += offset;
        }

        // Ensure the bounding box does not go out of bounds
        if (boxes[i].q1_x < 0) boxes[i].q1_x = 0;
        if (boxes[i].q1_y < 0) boxes[i].q1_y = 0;
        if (boxes[i].q2_x > 1) boxes[i].q2_x = 1; // Assuming normalized coordinates
        if (boxes[i].q2_y > 1) boxes[i].q2_y = 1;
    }
}

void adjust_boxes(BoundingBox* boxes, int num_boxes) {
    for (int i = 0; i < num_boxes; ++i) {
        float regw = boxes[i].q2_x - boxes[i].q1_x;
        float regh = boxes[i].q2_y - boxes[i].q1_y;

        boxes[i].q1_x += boxes[i].dx1 * regw;
        boxes[i].q1_y += boxes[i].dy1 * regh;
        boxes[i].q2_x += boxes[i].dx2 * regw;
        boxes[i].q2_y += boxes[i].dy2 * regh;
    }
}

void image_resize(float* input, float* output, u32 height, u32 width, float scale_factor){
    int in_width  = width;
    int in_height = height;

    int out_width  = round(in_width * scale_factor);
    int out_height = round(in_height * scale_factor);
    // printf("in_width(%d), in_height(%d), out_width(%d), out_height(%d) \n", in_width, in_height, out_width, out_height);

    for (int i = 0; i < (in_width * in_height); i++) {
        output[i] = 0.0f;
    }

    for (int y = 0; y < out_height; y++) {
        for (int x = 0; x < out_width; x++) {

            float gx = (float)(x * (in_width - 1)) / (out_width - 1);
            float gy = (float)(y * (in_height - 1)) / (out_height - 1);
            int gxi = (int)gx;
            int gyi = (int)gy;
            float fx = gx - gxi;
            float fy = gy - gyi;

            if (gxi >= in_width - 1) gxi = in_width - 2;
            if (gyi >= in_height - 1) gyi = in_height - 2;

            float top_left = input[gyi * in_width + gxi];
            float top_right = input[gyi * in_width + (gxi + 1)];
            float bottom_left = input[(gyi + 1) * in_width + gxi];
            float bottom_right = input[(gyi + 1) * in_width + (gxi + 1)];

            float top = top_left * (1 - fx) + top_right * fx;
            float bottom = bottom_left * (1 - fx) + bottom_right * fx;
            output[y * out_width + x] = top * (1 - fy) + bottom * fy;
        }
    }
}




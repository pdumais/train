#ifndef CONSTANTS_H
#define CONSTANTS_H

#define VIDEO_WIDTH 1920
#define VIDEO_HEIGHT 1080

#define TRACK_WIDTH 60
#define WAGON_WIDTH 130
#define LOCO_WIDTH 200
#define WAGON_HEIGHT 40
#define LOCO_HEIGHT 40
#define CROSSROAD_RADIUS 200
#define SPLITTER_RADIUS 200
#define DETECTION_SQUARE_SIZE 400
#define ANNOTATION_DETECTION_SQUARE_SIZE 600
#define DETECT_RECT 20
#define MAXIMUM_WAGON_CONNECTION_SIZE 50

#define SPEED_SLOW 75
#define SPEED_NORMAL 120
#define SPEED_FAST 200

#define MAX_RELAY 10
#define RELAY_RECORD_SIZE 10
#define RELAY_MAX_TIME_ABUSE 15

// When auto-assiging relays, we will reserve 8 first ones for splitters
// This is to prevent a case where a false crossroad is detected and we end up
// using a splitter relay for it and we would burn the coil
#define FIRST_CROSSROAD_RELAY 'i'

#define CROSSING_SOUND "/home/pat/projects/train/data/crossing.wav"

/////////////////////////////////////

#define DEBUGIMG8(img) {QImage image(img.data,img.cols,img.rows, static_cast<int>(img.step), QImage::Format_Grayscale8); DisplayService::debugPixmap->setPixmap(QPixmap::fromImage(image).copy());}

#define HYPOTHENUS(p) sqrt((p.x()*p.x())+(p.y()*p.y()))

#define MARKER_TYPE_CROSSING 1

#endif // CONSTANTS_H


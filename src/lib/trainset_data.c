#include <trainset_data.h>
#include <user.h>
#include <shared.h>

int switch_number_to_node_number(int switch_number) {
    int node_number = -1;
    if (switch_number > 0 && switch_number <= 18) {
        node_number = 80 + (switch_number - 1) * 2;
    }
    if (switch_number >= 153 && switch_number <= 156) {
        node_number = 116 + (switch_number - 153) * 2;
    }

    if (node_number == -1) u_error("Failed to convert switch_number %d", switch_number);

    return node_number;
}

void init_trainset(Train_Node *trainset) {
    u_memset(trainset, 0, TRAIN_MAX*sizeof(Train_Node));
    trainset[0].number = 1;
    trainset[0].curr_level = 0;
    trainset[0].curr_speed = 8;
    trainset[0].curr_velocity = 270;
    trainset[0].curr_stopping_distance = 145;
    trainset[0].last_read_time = 0;
    trainset[0].time_elapsed = 0;
    // calibrated
    trainset[0].measurement[TT_TRACK_A].speed[0] = 8;
    trainset[0].measurement[TT_TRACK_A].speed[1] = 11;
    trainset[0].measurement[TT_TRACK_A].speed[2] = 14;
    trainset[0].measurement[TT_TRACK_A].velocity[0] = 270;
    trainset[0].measurement[TT_TRACK_A].velocity[1] = 450;
    trainset[0].measurement[TT_TRACK_A].velocity[2] = 630;
    trainset[0].measurement[TT_TRACK_A].stopping_distance[0] = 145;
    trainset[0].measurement[TT_TRACK_A].stopping_distance[1] = 530;
    trainset[0].measurement[TT_TRACK_A].stopping_distance[2] = 1280;
    trainset[0].measurement[TT_TRACK_B].speed[0] = 8;
    trainset[0].measurement[TT_TRACK_B].speed[1] = 11;
    trainset[0].measurement[TT_TRACK_B].speed[2] = 14;
    trainset[0].measurement[TT_TRACK_B].velocity[0] = 260;
    trainset[0].measurement[TT_TRACK_B].velocity[1] = 445;
    trainset[0].measurement[TT_TRACK_B].velocity[2] = 620;
    trainset[0].measurement[TT_TRACK_B].stopping_distance[0] = 140;
    trainset[0].measurement[TT_TRACK_B].stopping_distance[1] = 530;
    trainset[0].measurement[TT_TRACK_B].stopping_distance[2] = 1280;

    trainset[1].number = 24;
    trainset[1].curr_level = 0;
    trainset[1].curr_speed = 8;
    trainset[1].curr_velocity = 220;
    trainset[1].curr_stopping_distance = 140;
    trainset[1].last_read_time = 0;
    trainset[1].time_elapsed = 0;
    trainset[1].measurement[TT_TRACK_A].speed[0] = 8;
    trainset[1].measurement[TT_TRACK_A].speed[1] = 11;
    trainset[1].measurement[TT_TRACK_A].speed[2] = 14;
    trainset[1].measurement[TT_TRACK_A].velocity[0] = 220;
    trainset[1].measurement[TT_TRACK_A].velocity[1] = 410;
    trainset[1].measurement[TT_TRACK_A].velocity[2] = 590;
    trainset[1].measurement[TT_TRACK_A].stopping_distance[0] = 140;
    trainset[1].measurement[TT_TRACK_A].stopping_distance[1] = 550;
    trainset[1].measurement[TT_TRACK_A].stopping_distance[2] = 1220;
    trainset[1].measurement[TT_TRACK_B].speed[0] = 8;
    trainset[1].measurement[TT_TRACK_B].speed[1] = 11;
    trainset[1].measurement[TT_TRACK_B].speed[2] = 14;
    trainset[1].measurement[TT_TRACK_B].velocity[0] = 210;
    trainset[1].measurement[TT_TRACK_B].velocity[1] = 400;
    trainset[1].measurement[TT_TRACK_B].velocity[2] = 580;
    trainset[1].measurement[TT_TRACK_B].stopping_distance[0] = 180;
    trainset[1].measurement[TT_TRACK_B].stopping_distance[1] = 600;
    trainset[1].measurement[TT_TRACK_B].stopping_distance[2] = 1350;

    trainset[2].number = 58;
    trainset[2].curr_level = 0;
    trainset[2].curr_speed = 8;
    trainset[2].curr_velocity = 200;
    trainset[2].curr_stopping_distance = 130;
    trainset[2].last_read_time = 0;
    trainset[2].time_elapsed = 0;
    trainset[2].measurement[TT_TRACK_A].speed[0] = 8;
    trainset[2].measurement[TT_TRACK_A].speed[1] = 11;
    trainset[2].measurement[TT_TRACK_A].speed[2] = 14;
    trainset[2].measurement[TT_TRACK_A].velocity[0] = 200;
    trainset[2].measurement[TT_TRACK_A].velocity[1] = 380;
    trainset[2].measurement[TT_TRACK_A].velocity[2] = 590;
    trainset[2].measurement[TT_TRACK_A].stopping_distance[0] = 130;
    trainset[2].measurement[TT_TRACK_A].stopping_distance[1] = 520;
    trainset[2].measurement[TT_TRACK_A].stopping_distance[2] = 1190;
    trainset[2].measurement[TT_TRACK_B].speed[0] = 8;
    trainset[2].measurement[TT_TRACK_B].speed[1] = 11;
    trainset[2].measurement[TT_TRACK_B].speed[2] = 14;
    trainset[2].measurement[TT_TRACK_B].velocity[0] = 190;
    trainset[2].measurement[TT_TRACK_B].velocity[1] = 370;
    trainset[2].measurement[TT_TRACK_B].velocity[2] = 580;
    trainset[2].measurement[TT_TRACK_B].stopping_distance[0] = 120;
    trainset[2].measurement[TT_TRACK_B].stopping_distance[1] = 500;
    trainset[2].measurement[TT_TRACK_B].stopping_distance[2] = 1160;

    trainset[3].number = 74;
    trainset[3].curr_level = 0;
    trainset[3].curr_speed = 8;
    trainset[3].curr_velocity = 380;
    trainset[3].curr_stopping_distance = 420;
    trainset[3].last_read_time = 0;
    trainset[3].time_elapsed = 0;
    trainset[3].measurement[TT_TRACK_A].speed[0] = 8;
    trainset[3].measurement[TT_TRACK_A].speed[1] = 11;
    trainset[3].measurement[TT_TRACK_A].speed[2] = 14;
    trainset[3].measurement[TT_TRACK_A].velocity[0] = 380;
    trainset[3].measurement[TT_TRACK_A].velocity[1] = 545;
    trainset[3].measurement[TT_TRACK_A].velocity[2] = 650;
    trainset[3].measurement[TT_TRACK_A].stopping_distance[0] = 450;
    trainset[3].measurement[TT_TRACK_A].stopping_distance[1] = 650;
    trainset[3].measurement[TT_TRACK_A].stopping_distance[2] = 780;
    trainset[3].measurement[TT_TRACK_B].speed[0] = 8;
    trainset[3].measurement[TT_TRACK_B].speed[1] = 11;
    trainset[3].measurement[TT_TRACK_B].speed[2] = 14;
    trainset[3].measurement[TT_TRACK_B].velocity[0] = 380;
    trainset[3].measurement[TT_TRACK_B].velocity[1] = 540;
    trainset[3].measurement[TT_TRACK_B].velocity[2] = 640;
    trainset[3].measurement[TT_TRACK_B].stopping_distance[0] = 460;
    trainset[3].measurement[TT_TRACK_B].stopping_distance[1] = 660;
    trainset[3].measurement[TT_TRACK_B].stopping_distance[2] = 800;

    trainset[4].number = 78;
    trainset[4].curr_level = 0;
    trainset[4].curr_speed = 8;
    trainset[4].curr_velocity = 180;
    trainset[4].curr_stopping_distance = 100;
    trainset[4].last_read_time = 0;
    trainset[4].time_elapsed = 0;
    trainset[4].measurement[TT_TRACK_A].speed[0] = 8;
    trainset[4].measurement[TT_TRACK_A].speed[1] = 11;
    trainset[4].measurement[TT_TRACK_A].speed[2] = 14;
    trainset[4].measurement[TT_TRACK_A].velocity[0] = 180;
    trainset[4].measurement[TT_TRACK_A].velocity[1] = 330;
    trainset[4].measurement[TT_TRACK_A].velocity[2] = 530;
    trainset[4].measurement[TT_TRACK_A].stopping_distance[0] = 100;
    trainset[4].measurement[TT_TRACK_A].stopping_distance[1] = 375;
    trainset[4].measurement[TT_TRACK_A].stopping_distance[2] = 880;
    // calibrated
    trainset[4].measurement[TT_TRACK_B].speed[0] = 8;
    trainset[4].measurement[TT_TRACK_B].speed[1] = 11;
    trainset[4].measurement[TT_TRACK_B].speed[2] = 14;
    trainset[4].measurement[TT_TRACK_B].velocity[0] = 195;
    trainset[4].measurement[TT_TRACK_B].velocity[1] = 350;
    trainset[4].measurement[TT_TRACK_B].velocity[2] = 520;
    trainset[4].measurement[TT_TRACK_B].stopping_distance[0] = 120;
    trainset[4].measurement[TT_TRACK_B].stopping_distance[1] = 400;
    trainset[4].measurement[TT_TRACK_B].stopping_distance[2] = 900;

    trainset[5].number = 79;
    trainset[5].curr_level = 0;
    trainset[5].curr_speed = 8;
    trainset[5].curr_velocity = 250;
    trainset[5].curr_stopping_distance = 165;
    trainset[5].last_read_time = 0;
    trainset[5].time_elapsed = 0;
    trainset[5].measurement[TT_TRACK_A].speed[0] = 8;
    trainset[5].measurement[TT_TRACK_A].speed[1] = 11;
    trainset[5].measurement[TT_TRACK_A].speed[2] = 14;
    trainset[5].measurement[TT_TRACK_A].velocity[0] = 250;
    trainset[5].measurement[TT_TRACK_A].velocity[1] = 450;
    trainset[5].measurement[TT_TRACK_A].velocity[2] = 670;
    trainset[5].measurement[TT_TRACK_A].stopping_distance[0] = 165;
    trainset[5].measurement[TT_TRACK_A].stopping_distance[1] = 540;
    trainset[5].measurement[TT_TRACK_A].stopping_distance[2] = 1220;
    trainset[5].measurement[TT_TRACK_B].speed[0] = 8;
    trainset[5].measurement[TT_TRACK_B].speed[1] = 11;
    trainset[5].measurement[TT_TRACK_B].speed[2] = 14;
    trainset[5].measurement[TT_TRACK_B].velocity[0] = 245;
    trainset[5].measurement[TT_TRACK_B].velocity[1] = 440;
    trainset[5].measurement[TT_TRACK_B].velocity[2] = 650;
    trainset[5].measurement[TT_TRACK_B].stopping_distance[0] = 180;
    trainset[5].measurement[TT_TRACK_B].stopping_distance[1] = 560;
    trainset[5].measurement[TT_TRACK_B].stopping_distance[2] = 1230;
}
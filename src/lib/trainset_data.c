#include <trainset_data.h>
#include <shared.h>

void init_trainset(Train_Node *trainset) {
    u_memset(trainset, 0, TRAIN_MAX*sizeof(Train_Node));
    trainset[0];
}
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    FILE* config;
    char group[3][1024];

    config = fopen("opt.conf", "r");
    printf("ok\n");
    fgets(group[0], 20, config);
    fgets(group[1], 20, config);
    fgets(group[2], 20, config);/*
    fscanf(config, "%s", group[0]);
    fscanf(config, "%s", group[1]);
    fscanf(config, "%s", group[2]);*/
    fclose(config);

    printf("first row: %s\nSecond row: %s\nThird row: %s\n", group[0], group[1], group[2]);

    return 0;
}

#include "detection.h"

void init() {
    cur_data= (struct data*) malloc(sizeof(struct data));
    prev_data= (struct data*) malloc(sizeof(struct data));
    printf("Initialized..\n");

    // sample
    cur_data->ewma = 0;
    cur_data->packet_count = 0;
    prev_data->ewma = 100;
    prev_data->packet_count = 0;
}

void *receive() {
    printf("Start receiving...\n");
    int recvfd = -1, recvlen = 0;
    char skbuf[1514];
    struct tcphdr *tcp_recvpkt;    

    memset(skbuf, 0 ,sizeof(skbuf));
    tcp_recvpkt = (struct tcphdr*) malloc(sizeof(struct tcphdr));

    if ((recvfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) == -1) {
        perror("Error with recvfd...\n");
    }
    while (1) {
        recvlen = recv(recvfd, skbuf, sizeof(skbuf), 0);
        if (recvlen > 0) {
            tcp_recvpkt = (struct tcphdr*) (skbuf + ETHER_HDR_LEN + IP_HDR_LEN);
            if (tcp_recvpkt->syn == 1) {
                packet_count++;
                // printf("package received!! SYN\n");
            } else {
                // printf("package received!! NOT SYN\n");
            }  
        }
    }
}

void detect() {
    timer++;
    assert(cur_data);
    assert(prev_data);
    cur_data->packet_count = packet_count;
    packet_count = 0;
    printf("%d %d\n", cur_data->packet_count, packet_count);

    // calculate
    cur_data->ewma = BETA * prev_data->ewma + (1 - BETA) * cur_data->packet_count;
    int signal = cur_data->packet_count >= (ALPHA + 1) * prev_data->ewma ? 1 : 0;
    prev_data->ewma = cur_data->ewma;
    // assert(signal);
    printf("prev_ewma : %f\n", prev_data->ewma);
    printf("Signal : %d\n", signal);
    if (signal) {
        signal_sum += 1;
    } else {
        signal_sum = 0;
    }
    if (timer - K + 1 < 0) {
        printf("Time interval : %d / Signal Sum : %d / Status : Pass\n", timer, signal_sum);
        return ;
    }
    if (signal_sum >= K) {
        printf("Time interval : %d / Signal Sum : %d / Status : SYN Flood Detected\n", timer, signal_sum);
    } else {
        printf("Time interval : %d / Signal Sum : %d / Status : OK\n", timer, signal_sum);
    }
    // printf("%f %d\n", cur_data->ewma, cur_data->packet_count);
}

void run() {
    int seconds = 0, s = -1, recvfd = -1, pd;
    struct sockaddr_in *sa;

    pthread_t tid;
    pd = pthread_create(&tid, NULL, receive, NULL);
    while(1) {
        // receive();
        sleep(1);
        seconds += 1;
        if (seconds % TIME_INTERVAL == 0) {
            detect();
        }
    }
}

void *thr_fn(void *arg) {

}

struct detection default_detection = {
    ._init = init,
    ._run = run,
    ._detect = detect,
};


int main(int argc, char const *argv[])
{
    default_detection._init();
    default_detection._run();

}
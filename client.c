#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio_ext.h>
#include "process.h" 

// Handle Ctrl C
void ctr_c_handler() {
    printf("\nGoodbye Hust!\n");
    exit(0);
}

int connectServer(int sockfd) {
    char test[BUFF_SIZE];
    char choice[2];
    char username[BUFF_SIZE];
    char password[BUFF_SIZE];
    char tmp[BUFF_SIZE];
    char tmp2[BUFF_SIZE];
    int signup = 0;

    while(1) {
        if(signup == 1){
            printf("|             **Sign up successful!**             |\n");
        }
        else if(signup == -2){
            printf("       **Error! Username already exists!**       \n");
        }

        printf("1. Register\n");
        printf("2. Login\n");
        __fpurge(stdin);
        fgets(choice, sizeof(choice), stdin);
        int check = choice[0] - '0';
        write(sockfd, choice, 2);
        switch (check) {
            case 1:
                printf("               **Register**               \n");
                printf("Username: ");
                __fpurge(stdin);
                fgets(username, sizeof(username), stdin);
                write(sockfd, username, BUFF_SIZE);
                read(sockfd, &test, 10);
                if(strcmp(test, "NotOK") == 0) {
                    signup = -2;
                    break;
                } else {
                    printf("Password: ");
                    __fpurge(stdin);
                    fgets(password,sizeof(password), stdin);
                    while(strlen(password) < 6){
                        printf("Password length must be greater than or equal to 6 characters!\n");
                        printf("Password: ");
                        __fpurge(stdin);
                        fgets(password,sizeof(password), stdin);
                    }
                    write(sockfd, password, BUFF_SIZE);
                }
                signup = 1;
                break;
            case 2:
                printf("                **Login**               \n");
                printf("Username: ");
                __fpurge(stdin);
                fgets(username,sizeof(username), stdin);
                write(sockfd, username, BUFF_SIZE);
                read(sockfd, &test, 10);
                if(strcmp(test, "NotOK") == 0) {
                    printf("Not found!\n");
                    break;
                } else {
                    strcpy(tmp, username);
                    printf("Password: ");
                    __fpurge(stdin);
                    fgets(password,sizeof(password), stdin);
                    write(sockfd, password, BUFF_SIZE);
                    read(sockfd, &test, BUFF_SIZE);
                    while(strcmp(test, "CorrectPass") != 0){
                        printf("Error! Wrong password!\n");
                        printf("Password: ");
                        __fpurge(stdin);
                        fgets(password,sizeof(password), stdin);
                        write(sockfd, password, BUFF_SIZE);
                        read(sockfd, &test, BUFF_SIZE);
                    }
                }
                back:
                __fpurge(stdin);
                if(signup == -3) {
                    printf("       **Password changed successfully!**        \n");
                    printf("_________________________________________________\n");
                } else {
                    printf(" ____________Logged in successfully!______________ \n");
                }
                printf("1. Join waiting-room\n");
                printf("2. Change password\n");
                printf("3. Quit game\n");
                printf("********************\n");
                __fpurge(stdin);
                fgets(choice, sizeof(choice), stdin);
                while(strlen(choice) == 0 || choice[0] < '1' || choice[0] > '3'){
                    printf("Input from 1 to 3");
                    __fpurge(stdin);
                    fgets(choice,sizeof(choice), stdin);
                }
                int check2 = choice[0] - '0';
                char new_password[BUFF_SIZE], choose[BUFF_SIZE];
                switch(check2) {
                    case 1:
                        write(sockfd, choice, 2);
                        while(1) {
                            printf("Please wait another player...\n");
                            printf("Press [S] to start game\n");
                            printf("Press [Q] to quit game!\n");
                            __fpurge(stdin);
                            fgets(choose, sizeof(choose), stdin);
                            if((strcmp(choose, "Q") == 0) || (strcmp(choose, "q") == 0)) {
                                printf("Bye!");
                                exit(0);
                            }
                            write(sockfd, choose, 2);
                            int data = read(sockfd, &tmp2, BUFF_SIZE);
                            if(data == 0) {
                                break;
                            }
                            tmp2[data] = '\0';
                            // if(strcmp(tmp2, "Play") == 0) {
                            //     return 1;
                            // } else if(strcmp(tmp2, "Quit") == 0){
                            //     return 0;
                            //     break;
                            // } else {
                            //     printf("Please waiting...");
                            // }
                            printf("%s\n", tmp2);
                        }
                        break;
                    case 2:
                        write(sockfd, choice, 2);
                        printf("New password: ");
                        __fpurge(stdin);
                        fgets(new_password, sizeof(new_password), stdin);
                        while(strlen(new_password) < 6){
                            printf("Password length must be greater than or equal to 6 characters!\n");
                            printf("New password: ");
                            __fpurge(stdin);
                            fgets(new_password,sizeof(new_password), stdin);
                        }
                        write(sockfd, new_password, BUFF_SIZE);
                        signup = -3;
                        goto back;
                        break;
                    case 3:
                        signup = 0;
                        write(sockfd, choice, 2);
                        return 0;
                        write(sockfd, choice, 2);                      
                        goto back;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in servaddr;
    char *SERV_ADDR = argv[1];

    signal(SIGINT, ctr_c_handler);

    if(argc != 3) {
        printf("Wrong input format!\n");
        exit(1);
    }
    
    if(checkValidPort(argv[2]) == 0) {
        printf("Invalid port!\n");
        exit(1);
    }

    int SERV_PORT = atoi(argv[2]);

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error");
        exit(0);
    }
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERV_ADDR);

    if(inet_pton(AF_INET, SERV_ADDR, &servaddr.sin_addr) <= 0) { 
		perror("Error");
		exit(0);
	} 

	if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) { 
		perror("Error");
		exit(0);
	} 
    
    int checkConnect2Server = connectServer(sock);
    if(checkConnect2Server == 0) {
        return 0;
    }

    printf("Game will start after: 5 seconds\n");
    sleep(1);
    printf("Game will start after: 4 seconds\n");
    sleep(1);
    printf("Game will start after: 3 seconds\n");
    sleep(1);
    printf("Game will start after: 2 seconds\n");
    sleep(1);
    printf("Game will start after: 1 seconds\n");
    sleep(1);

    return 0;
}
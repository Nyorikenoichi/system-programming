/* 
 * SPLPv1.c
 * The file is part of practical task for System programming course. 
 * This file contains validation of SPLPv1 protocol. 
 */


/*
  Мальцев Максим
  13 группа
*/



/*
---------------------------------------------------------------------------------------------------------------------------
# |      STATE      |         DESCRIPTION       |           ALLOWED MESSAGES            | NEW STATE | EXAMPLE
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
1 | INIT            | initial state             | A->B     CONNECT                      |     2     |
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
2 | CONNECTING      | client is waiting for con-| A<-B     CONNECT_OK                   |     3     |
  |                 | nection approval from srv |                                       |           |                      
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
3 | CONNECTED       | Connection is established | A->B     GET_VER                      |     4     |                     
  |                 |                           |        -------------------------------+-----------+----------------------
  |                 |                           |          One of the following:        |     5     |                      
  |                 |                           |          - GET_DATA                   |           |                      
  |                 |                           |          - GET_FILE                   |           |                      
  |                 |                           |          - GET_COMMAND                |           |
  |                 |                           |        -------------------------------+-----------+----------------------
  |                 |                           |          GET_B64                      |     6     |                      
  |                 |                           |        ------------------------------------------------------------------
  |                 |                           |          DISCONNECT                   |     7     |                                 
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
4 | WAITING_VER     | Client is waiting for     | A<-B     VERSION ver                  |     3     | VERSION 2                     
  |                 | server to provide version |          Where ver is an integer (>0) |           |                      
  |                 | information               |          value. Only a single space   |           |                      
  |                 |                           |          is allowed in the message    |           |                      
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
5 | WAITING_DATA    | Client is waiting for a   | A<-B     CMD data CMD                 |     3     | GET_DATA a GET_DATA 
  |                 | response from server      |                                       |           |                      
  |                 |                           |          CMD - command sent by the    |           |                      
  |                 |                           |           client in previous message  |           |                      
  |                 |                           |          data - string which contains |           |                      
  |                 |                           |           the following allowed cha-  |           |                      
  |                 |                           |           racters: small latin letter,|           |                     
  |                 |                           |           digits and '.'              |           |                      
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
6 | WAITING_B64_DATA| Client is waiting for a   | A<-B     B64: data                    |     3     | B64: SGVsbG8=                    
  |                 | response from server.     |          where data is a base64 string|           |                      
  |                 |                           |          only 1 space is allowed      |           |                      
--+-----------------+---------------------------+---------------------------------------+-----------+----------------------
7 | DISCONNECTING   | Client is waiting for     | A<-B     DISCONNECT_OK                |     1     |                      
  |                 | server to close the       |                                       |           |                      
  |                 | connection                |                                       |           |                      
---------------------------------------------------------------------------------------------------------------------------

IN CASE OF INVALID MESSAGE THE STATE SHOULD BE RESET TO 1 (INIT)

*/


#include "splpv1.h"
#include "string.h"




/* FUNCTION:  validate_message
 * 
 * PURPOSE:  
 *    This function is called for each SPLPv1 message between client 
 *    and server
 * 
 * PARAMETERS:
 *    msg - pointer to a structure which stores information about 
 *    message
 * 
 * RETURN VALUE:
 *    MESSAGE_VALID if the message is correct 
 *    MESSAGE_INVALID if the message is incorrect or out of protocol 
 *    state
 */
int state = 1;

enum test_status validate_message( struct Message *msg )
{
    char* text = msg -> text_message;
    int len = strlen(text);
    enum Direction dir = msg->direction;
    
    if (strcmp(text, "CONNECT") == 0 && dir == A_TO_B && state == 1) {
        state = 2;
        return MESSAGE_VALID;
    }

    if (strcmp(text, "CONNECT_OK") == 0 && dir == B_TO_A && state == 2) {
        state = 3;
        return MESSAGE_VALID;
    }

    if (strcmp(text, "GET_VER") == 0 && dir == A_TO_B && state == 3) {
        state = 4;
        return MESSAGE_VALID;
    }

    if ((strcmp(text, "GET_DATA") == 0 || strcmp(text, "GET_COMMAND") == 0 || strcmp(text, "GET_FILE") == 0) && dir == A_TO_B && state == 3) {
        state = 5;
        return MESSAGE_VALID;
    }

    if (strcmp(text, "GET_B64") == 0 && dir == A_TO_B && state == 3) {
        state = 6;
        return MESSAGE_VALID;
    }

    if (strcmp(text, "DISCONNECT") == 0 && dir == A_TO_B && state == 3) {
        state = 7;
        return MESSAGE_VALID;
    }

    if (strncmp(text, "VERSION ", 8) == 0 && dir == B_TO_A && state == 4) {
        for(int i = 8; i < len; i++)
            if (text[i] < '0' || text[i] > '9') {
                state = 1;
                return MESSAGE_INVALID;
            }

        state = 3;
        return MESSAGE_VALID;
    }

    if ((strncmp(text, "GET_DATA", 8) == 0 || strncmp(text, "GET_COMMAND", 11) == 0 || strncmp(text, "GET_FILE", 8) == 0) && dir == B_TO_A && state == 5) {
        int i = 8;
        while (text[i] != ' ')
            i++;
        i++;

        while (text[i] != ' ') {
            if (!(text[i] >= 'a' && text[i] <= 'z') && !(text[i] >= '0' && text[i] <= '9') && text[i] != '.') {
                state = 1;
                return MESSAGE_INVALID;
            }
            i++;
        }
        i++;
        
        int betweenGet = i;
        while (i < len) {
            if (text[i] != text[i - betweenGet]) {
                state = 1;
                return MESSAGE_INVALID;
            }
            i++;
        }
        
        state = 3;
        return MESSAGE_VALID;
    }

    if (strncmp(text, "B64: ", 5) == 0 && dir == B_TO_A && state == 6) {
        if ((len - 5) % 4 != 0) {
            state = 1;
            return MESSAGE_INVALID;
        }

        for (int i = 5; i < len; i++)
            if (!(text[i] >= 'a' && text[i] <= 'z') && !(text[i] >= 'A' && text[i] <= 'Z') && !(text[i] >= '0' && text[i] <= '9') && 
               text[i] != '+' && text[i] != '/' && (i >= len - 2 ? text[i] != '=' : 1)) {
                state = 1;
                return MESSAGE_INVALID;
            }

        state = 3;
        return MESSAGE_VALID;
    }

    if (strcmp(text, "DISCONNECT_OK") == 0 && dir == B_TO_A && state == 7) {
        state = 1;
        return MESSAGE_VALID;
    }

    state = 1;
    return MESSAGE_INVALID;     
}

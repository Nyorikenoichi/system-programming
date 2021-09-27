/*************************************************************************
   LAB 1                                                                

    Edit this file ONLY!

*************************************************************************/

#include "dns.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define A 55021
#define B 75011
#define C 85021
#define FIRST 131
#define SIZE 12837

extern unsigned int getNumOfLines(FILE*);

typedef struct _Vertex
{
    char* host_name[201];
    IPADDRESS ip;
    struct Vertex* next_vertex;
}Vertex;

unsigned int Get_hash(const char* host_name) 
{
    unsigned int hash = FIRST;
    while (*host_name) 
    {
        hash = (hash * A) ^ (*host_name * B);
        ++host_name;
    }
    hash %= C;
    return hash % SIZE;
}

void AddToTable(DNSHandle hDNS, char* host_name, IPADDRESS ip)
{
    Vertex* v;
    Vertex* next_v;
    unsigned int index = Get_hash(host_name);
    v = (Vertex*) malloc(sizeof(Vertex));
    next_v = ((Vertex**)hDNS)[index];
    strcpy_s((char *)v->host_name, 201, host_name);
    v->ip = ip;
    v->next_vertex = (struct Pair*)next_v;
    ((Vertex**)hDNS)[index] = (Vertex*) v;
}
DNSHandle InitDNS( )
{
    DNSHandle hDNS = (unsigned int)(Vertex*)calloc(SIZE, sizeof(Vertex));
    if ((Vertex*)hDNS != NULL)
        return hDNS;
    return INVALID_DNS_HANDLE;
}

void LoadHostsFile(DNSHandle hDNS, const char* hostsFilePath)
{
    FILE* fInput = NULL;

    fInput = fopen(hostsFilePath, "r");

    unsigned int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0;
    char* string = (char*)malloc(201);

    if (NULL == fInput)
        return;
    while (fscanf_s(fInput, "%d.%d.%d.%d %s", &ip1, &ip2, &ip3, &ip4, string, 200) != EOF)
    {
        IPADDRESS ip = (ip1 & 0xFF) << 24 |
            (ip2 & 0xFF) << 16 |
            (ip3 & 0xFF) << 8 |
            (ip4 & 0xFF);
        AddToTable(hDNS, string, ip);
    }
    free(string);
    fclose(fInput);
}

IPADDRESS DnsLookUp( DNSHandle hDNS, const char* hostName )
{
    unsigned int index = Get_hash(hostName);
    Vertex* v = ((Vertex**)hDNS)[index];
    while (v != NULL && strcmp(v->host_name, hostName) != 0)
    {
        v = v->next_vertex;
    }
    if (v != NULL)
        return v->ip;
    return INVALID_IP_ADDRESS;
}

void ShutdownDNS( DNSHandle hDNS )
{
    for (int i = 0; i < SIZE; i++) 
    {
        Vertex* v = ((Vertex**)hDNS)[i];
        Vertex* v_next = NULL;
        while (v != NULL)
        {
            v_next = (Vertex*)v->next_vertex;
            free(v);
            v = v_next;
        }
    }
    if ((Vertex*)hDNS != NULL)
        free((Vertex*)hDNS);
}
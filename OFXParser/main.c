//
//  main.c
//  OFXParser
//
//  Created by Todd Fearn on 4/8/14.
//  Copyright (c) 2014 iData Corporation. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

int fileIndex = 0;

char *getToken(char *buffer) {
    char *token = calloc(1, 4196);
    char *tokenPtr = token;
    
    buffer = buffer + fileIndex;
    
    if(*buffer == '<') {
        *token++ = *buffer++;
        fileIndex++;
    }

    while(*buffer != 0 && *buffer != '<') {
        if(*buffer == '\n' || *buffer == '\r') {
            buffer++;
            fileIndex++;
            continue;
        }
        *token++ = *buffer++;
        fileIndex++;
    }
    *token = 0;
    
    if(!strlen(tokenPtr))
        return NULL;

    return tokenPtr;
}

int doesTokenHaveValue(char *token) {
    
    // find the >
    char *tokenPtr = token;
    while(*tokenPtr != 0 && *tokenPtr != '>')
        tokenPtr++;
    if(*tokenPtr == 0)
        return 0;
    
    // Skip spaces after >
    tokenPtr++;
    while(*tokenPtr == ' ' && *tokenPtr != 0)
        tokenPtr++;
    if(*tokenPtr == 0)
        return 0;
    
    // We have no value
    if(*tokenPtr == '<')
        return 0;
    
    // Are we done?
    if(*tokenPtr == '>' && *(tokenPtr+1) == 0)
        return 0;
    
    return 1;
}

int isTokenACloser(char *token) {
    if(*token == '<' && *(token+1) == '/')
        return 1;
    else
        return 0;
}

char *trimToken(char *token) {
    char *trimmed = calloc(1, 512);
    char *trimmedPtr = trimmed;
    
    while(*token != 0) {
        if(*token != '<' && *token != '>' && *token != '/')
            *trimmedPtr++ = *token++;
        else
            token++;
    }
    *trimmedPtr = 0;
    
    return trimmed;
}

int compareOpenCloseTokens(char *openToken, char *closeToken) {

    char *open = trimToken(openToken);
    char *close = trimToken(closeToken);
    
    int compare = strcmp(open, close);
    
    free(open);
    free(close);
    
    return compare;
}

int main(int argc, const char * argv[])
{
    FILE *fpIn = fopen(argv[1] , "rb" );
    if(!fpIn) {
        perror("Error reading file");
        return 1;
    }
    
    fseek(fpIn , 0L , SEEK_END);
    long lSize = ftell(fpIn);
    rewind(fpIn);
    
    char *buffer = calloc(1, lSize+1);
    if(!buffer) {
        fclose(fpIn);
        fputs("memory alloc fails",stderr);
        return 1;
    }
    
    if(1!=fread( buffer , lSize, 1 , fpIn)) {
        fclose(fpIn);
        free(buffer);
        fputs("entire read fails",stderr);
        return 1;
    }
    
    FILE *fpOut = fopen(argv[2], "w");

    int line = 0;
    int spaces = 0;
    char *previousToken = calloc(1, 1024);
    while(1) {
        char *token = getToken(buffer);
        if(! token)
            break;
        
        if(line < 2) {
            line++;
            
            fprintf(fpOut, "%s\n", token);
            
            strcpy(previousToken, token);
            free(token);
            continue;
        }
        

        // Is the token a closer? </example>
        if(isTokenACloser(token)) {
            // Is the current token a closer to the previous?
            if(compareOpenCloseTokens(previousToken, token) == 0)
                spaces = spaces;
            else
                spaces -= 4;
        }
        else if(doesTokenHaveValue(token)) {
            if(isTokenACloser(previousToken))
                spaces = spaces;
            else if(doesTokenHaveValue(previousToken))
                spaces = spaces;
            else
                spaces += 4;
        }
        else {
            if(isTokenACloser(previousToken))
                spaces = spaces;
            else if(doesTokenHaveValue(previousToken))
                spaces = spaces;
            else
                spaces += 4;
        }
        
        char *spacesString = calloc(1, 1024);
        if(spaces > 0) {
            sprintf(spacesString, "%.*s", spaces, "                                                                               ");
        }
        
        fprintf(fpOut, "%s%s\n", spacesString, token);
        
        strcpy(previousToken, token);
        free(token);
        free(spacesString);
    }
    fclose(fpOut);

    
    fclose(fpIn);
    free(buffer);
    

    return 0;
}


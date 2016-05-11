#ifndef DIRENT_INCLUDED
#define DIRENT_INCLUDED

#ifdef _WIN32
/*

    Declaration of POSIX directory browsing functions and types for Win32.

    Author:  Kevlin Henney (kevlin@acm.org, kevlin@curbralan.com)
    History: Created March 1997. Updated June 2003.
    Rights:  See end of file.
    
*/

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef struct DIR DIR;

struct dirent
{
    char *d_name;
    unsigned char d_type;
};

enum {
    DT_BLK,
   // C'est un périphérique de blocs. 
    DT_CHR,
    //C'est un périphérique de caractères. 
    DT_DIR,
    //C'est un répertoire. 
    DT_FIFO,
    //C'est un tube nommé (FIFO). 
    DT_LNK,
    //C'est un lien symbolique. 
    DT_REG,
    //C'est un fichier ordinaire. 
    DT_SOCK,
    //C'est une socket de domaine Unix. 
    DT_UNKNOWN,
    //Le type de fichier est inconnu.
};

DIR           *opendir(const char *);
int           closedir(DIR *);
struct dirent *readdir(DIR *);
void          rewinddir(DIR *);

/*

    Copyright Kevlin Henney, 1997, 2003. All rights reserved.

    Permission to use, copy, modify, and distribute this software and its
    documentation for any purpose is hereby granted without fee, provided
    that this copyright and permissions notice appear in all copies and
    derivatives.
    
    This software is supplied "as is" without express or implied warranty.
    But that said, if there are any problems please get in touch.

*/

#ifdef __cplusplus
}
#endif // __cplusplus

#else // _WIN32
#	error "Dont include this when no in WIN32"
#endif // _WIN32

#endif //  DIRENT_INCLUDED

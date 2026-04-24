
// copyright Brandon W. Gordon

#include <iostream>
#include <fstream>

#include <windows.h>

#include "image_transfer7.h"

using namespace std;

static ofstream fout("image_transfer_cout.txt");

// pointer to file mapped memory
ibyte *pbData;

// handle to file mapped memory
HANDLE hFileMapping;

int activate_vision()
{
	char name[]="ImageMap"; // shared memory name
	
	// initialize the shared memory for image_view

	// create a file map to send data to the view_image application
    hFileMapping = CreateFileMappingA(INVALID_HANDLE_VALUE, // segment independant of disk file
                                 NULL,            // no security attributes
                                 PAGE_READWRITE,  // full access to memory
                                 (DWORD)0,        // less than 4Gb in size
                                 (DWORD)SMAX,     // size of buffer
                                 name);			  // name of the map

	if( hFileMapping != 0 ) {
		pbData = (ibyte *)MapViewOfFile(hFileMapping,
						FILE_MAP_ALL_ACCESS, 0, 0, 0);
	} else {
		fout << "\nshared memory error in activate_vision()";
		return 1; // error
	}

	if( !pbData ) {
		CloseHandle(hFileMapping);
		fout << "\nmapping error in activate_vision()";
		return 1; // error
	}
	
	i2byte *p_node;

	p_node = (i2byte *)pbData;

	// start with node A owning token
	*p_node = NODE_A;

	return 0;
}


int view_rgb_image(image &a)
{
	size_t size;
	int height, width;
	int i, n_timeout;

	width = a.width; 
	height = a.height;
	
	// check if image width is a multiple of 4
	if( (width % 4) > 0 ) {
		fout << "\nerror in view_rgb_image(...) - image width is not a multiple of 4";
		return 1; // error
	}

	// calculate the image size (in bytes)
	// 3 bytes per pixel for RGB image
	if(a.type == RGB_IMAGE) {
		size = (size_t)width * height * 3;
	} else {
		fout << "\nerror in view_image - image must be RGB image type";
		return 1;
	}

	// copy memory from the image to the file mapped memory
	
	ibyte *p_data;
	i2byte *p_width, *p_height, *p_node;
	
	p_data = pbData;
	
	p_node = (i2byte *)p_data; // pointer to node number that currently owns token
	p_data += sizeof(i2byte); // move on to next variable

	// return immediately if node A doesn't own the shared 
	// memory token
	if ( *p_node != NODE_A ) return 0;

	p_width = (i2byte *)p_data; // pointer to image width
	*p_width = width; // set image width in shared memory
	p_data += sizeof(i2byte); // move on to next variable
	
	p_height = (i2byte *)p_data; // pointer to image height
	*p_height = height; // set image height in shared memory	
	p_data += sizeof(i2byte); // move on to next variable	

	// copy image data to shared memory after the width and height info
	if( ( size <= (SMAX-15) ) && (size > 0) ) {
		memcpy(p_data,a.pdata,size);
	} else {
		fout << "\nerror in view_rgb_image(...) - image too large";
	}

	// give token to node B (image_view) so it can access shared memory
	*p_node = NODE_B;

	return 0;
}


int save_rgb_image(char *file_name, image &a)
{
	size_t size;
	int height, width;

	width = a.width; 
	height = a.height;
	
	// check if image width is a multiple of 4
	if( (width % 4) > 0 ) {
		fout << "\nerror in save_rgb_image(...) - image width is not a multiple of 4";
		return 1; // error
	}

	if(a.type == RGB_IMAGE) {
		size = width * height * 3;
	} else {
		fout << "\nerror in save_image - image must be RGB image type";
		return 1;
	}
	
    HANDLE hf = CreateFileA(
            file_name, GENERIC_WRITE, FILE_SHARE_READ, NULL,
            CREATE_ALWAYS, NULL, NULL );

    if( hf == INVALID_HANDLE_VALUE ){
		fout << "\nINVALID_HANDLE_VALUE\n";
		return 0;
	}

    // write out the file header
    BITMAPFILEHEADER bfh;
    memset( &bfh, 0, sizeof( bfh ) );
    bfh.bfType = 'MB';
    bfh.bfSize = sizeof( bfh ) + size + sizeof( BITMAPINFOHEADER );
    bfh.bfOffBits = sizeof( BITMAPINFOHEADER ) + sizeof( BITMAPFILEHEADER );

    DWORD dwWritten = 0;
    WriteFile( hf, &bfh, sizeof( bfh ), &dwWritten, NULL );

    // write out the bitmap format
    BITMAPINFOHEADER bih;
    memset( &bih, 0, sizeof( bih ) );
    bih.biSize = sizeof( bih );
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 24;

    WriteFile( hf, &bih, sizeof( bih ), &dwWritten, NULL );

    // write out the bits themselves
    WriteFile( hf, a.pdata, size, &dwWritten, NULL );

    CloseHandle( hf );

	return 0;
}


int load_rgb_image(char *file_name, image &a)
{
	size_t size;
	int height, width;

	width = a.width; 
	height = a.height;

	// check if image width is a multiple of 4
	if( (width % 4) > 0 ) {
		fout << "\nerror in load_rgb_image(...) - image width is not a multiple of 4";
		return 1; // error
	}

    HANDLE hf = CreateFileA(
            file_name, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_ALWAYS, NULL, NULL );

    if( hf == INVALID_HANDLE_VALUE ){
		fout << "\nINVALID_HANDLE_VALUE\n";
		return 0;
	}

    // read the file header
    BITMAPFILEHEADER bfh;
    DWORD dwRead;
    ReadFile( hf, &bfh, sizeof( bfh ), &dwRead, NULL );

	int W,H;
	
    // read the bitmap format
    BITMAPINFOHEADER bih;
    ReadFile( hf, &bih, sizeof( bih ), &dwRead, NULL );
	W = (int)(bih.biWidth);
    H = (int)(bih.biHeight);
	
	// check for compatibility
	if( W != width || H != height ) return 1;
	
	if(a.type == RGB_IMAGE) {
		size = width * height * 3;
	} else {
		fout << "\nerror in load_rgb_image - image must be RGB image type";
		return 1;
	}
	
    // read the bits themselves
	ReadFile( hf, a.pdata, size, &dwRead, NULL );

    CloseHandle( hf );

	return 0;
}


int allocate_image(image &a)
// allocate image memory
{
	size_t size;

	// calculate the image size (in bytes)
	// 3 bytes per pixel for RGB (RGB)
	if( a.type == RGB_IMAGE ) { // 3 bytes/pixel (B-G-A)
		size = (size_t)a.width * a.height * 3;
	} else if( a.type == GREY_IMAGE ) {
		// 1 byte/pixel (0-255)
		size = (size_t)a.width * a.height;
	} else if( a.type == LABEL_IMAGE ) { // 2 bytes/pixel (0-65535)
		size = (size_t)a.width * a.height * 2;
	} else {
		fout << "\nerror in allocate_image - unknown image type";
		return 1;
	}

	// allocate size bytes of dynamic memory
	a.pdata = (ibyte *)malloc(size*sizeof(ibyte));

	if(a.pdata == NULL) {
		fout << "\nmemory allocation error in allocate_image";
		return 1;
	}

	return 0;
}


int free_image(image &a)
// free image memory
{
	if(a.pdata == NULL) {
		fout << "\nerror in free_image - memory already freed";
		return 1;
	}

	// free image memory so other functions can use it
	free(a.pdata);

	// set the pointer to NULL so we know the memory has been freed
	a.pdata = NULL; 
	
	return 0;
}


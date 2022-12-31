#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

// Struct for an image, containing its dimensions and pixel data
struct bitmap
{
	int width;
	int height;
	int *pixels;
};

const int DIB_HEADER_SIZE = 14;
const int BMP_HEADER_SIZE = 40;

// Make "byte" mean "unsigned char"
typedef unsigned char byte;


void bitmap_posterize(struct bitmap *bmp);

void bitmap_mirror(struct bitmap *bmp);

void bitmap_squash(struct bitmap *bmp);

void bitmap_to_grayscale(struct bitmap *bmp);
// Calculates the stride of a .bmp file.
// (The stride is how many bytes of memory a single row of
// the image requires.)
inline int bmp_file_stride(struct bitmap *bmp);

// Calculates the total size that a .bmp file for this bitmap
// would need (in bytes)
inline int bmp_file_size(struct bitmap *bmp);

// Opens the file with the given name and maps it into memory
// so that we can access its contents through pointers.
void *map_file_for_reading(char *filename);

// Opens (and creates if necessary) the file with the given name
// and maps it into memory so that we can access its contents
// through pointers. 
void *map_file_for_writing(char *filename, int file_size);

// Takes the contents of a bitmap file (bmp_file) and reads
// its data, filling in the struct bitmap pointed to by bmp.
// Returns 0 if everything worked, -1 if the file data isn't
// valid.
int read_bitmap(void *bmp_file, struct bitmap *bmp);
void write_bitmap(void *bmp_file, struct bitmap *bmp);
// Converts between a packed pixel (0xRRGGBB) and its components.
void rgb_to_pixel(int *p, int r, int g, int b);
void pixel_to_rgb(int p, int *r, int *g, int *b);

/* Please note: if your program has a main() function, then
 * the test programs given to you will not run (your main()
 * will override the test program's). When running a test,
 * make sure to either comment out or rename your main()
 * function!
 */

int main(int argc, char *argv[])
{  struct bitmap bmp1;
   char *filename;
   
   char *a = map_file_for_reading(argv[1]);
   read_bitmap(a, &bmp1);

   char choice[26];
   while(choice[0] != 'q'){

    printf("Menu: ");
    printf("\nG) Make GrayScale");
    printf("\nP) Posterize");
    printf("\nU) Squash");
    printf("\nM) Mirror");
    printf("\nS) Save");
    printf("\nQ) Quit");
    printf("\nWhat would you like to do?");
    scanf("%s", choice);

    if (choice[0] == 'g' ){
     printf("\nGrayscale selected\n");
     bitmap_to_grayscale(&bmp1);
       }

    else if (choice[0] == 'p'){
     printf("\nPosterize selected\n");
     bitmap_posterize(&bmp1);
    }


    else if (choice[0] == 'u'){
     printf("\nSquash selected\n");
     bitmap_squash(&bmp1);
    }


    else if (choice[0] == 'm'){
     printf("\nMirror selected\n");
     bitmap_mirror(&bmp1);
    }

    else if (choice[0] == 's'){
        printf("\nSave selected\n");
        printf("\nEnter filename:");
        scanf("%s", filename);
        printf("\nSaving to output.bmp");
        int file_size1 = bmp_file_size(&bmp1);
        char *b = map_file_for_writing(filename, file_size1);
        write_bitmap(b, &bmp1);
        printf("\nSaved!");

 }

 }

 if(choice[0] =='q'){
 printf("\nBye!");

 }

   return 0;
 }


int bmp_file_stride(struct bitmap *bmp)
{
    return (24 * bmp->width + 31) / 32 * 4;
}

int bmp_file_size(struct bitmap *bmp)
{
    int stride = bmp_file_stride(bmp);
    return DIB_HEADER_SIZE
        + BMP_HEADER_SIZE
        + stride * bmp->height;
}

void *map_file_for_reading(char *filename)

{
    struct stat statbuf;
    
    // A) Use open() to open the file for reading.
    

    int fd =  open( filename, O_RDONLY);
    if( fd == -1){
     perror(NULL);
     printf("There is an error");
     return NULL;
     }
    
    
    
    // B) Use fstat() to determine the size of the file.
   
   int fd1 = fstat( fd,  &statbuf);
   if(fd1 ==-1){
    perror(NULL);
    printf("There is an error");
    return NULL;
   }
   int st_size = statbuf.st_size;
   


    // C) Call mmap() to map the file into memory.
    
   
   char* p = mmap( NULL , st_size, PROT_READ, MAP_SHARED, fd, 0);
   if (p == MAP_FAILED){
    perror(NULL);
    printf("There is an error");
     return NULL;
   }


    // D) Close the file using close().
     close( fd);


    // E) Return the pointer returned from mmap().
    return p;
    
    


    // Default implementation: just returns NULL.
    
}

void *map_file_for_writing(char *filename, int file_size)
{
    // A) Use open() to open the file for writing.
    int fd =  open( filename, O_RDWR | O_CREAT,0644);
    if( fd == -1){
     perror(NULL);
     printf("There is an error");
     return NULL;
     }
   
    
    // B) Use ftruncate() to set the size of the file.
      int fd1 = ftruncate( fd,  file_size);
   if(fd1 ==-1){
     perror(NULL);
     printf("There is an error");
     return NULL;
   }


    // C) Call mmap() to map the file into memory.
   char* p = mmap( NULL , file_size, PROT_WRITE, MAP_SHARED, fd, 0);
   if (p == MAP_FAILED){
     perror(NULL);
     printf("There is an error");
     return NULL;
   }


    // D) Close the file using close().
    close(fd);


    // E) Return the pointer returned from mmap().
    return p;


    // Default implementation: just returns NULL.
    

}

int read_bitmap(void *bmp_file, struct bitmap *bmp)
{
    // Cast bmp_file to a byte * so we can access it
    // byte by byte.
    
    byte *file = (byte *) bmp_file;
    
    int w = *((int*) (file  + 18));
    int h = *((int *)(file + 22));

    int offset = *((int *)(file + 10));
    bmp->width = w;
    bmp->height = h;

    int pixelSize = h * w;
    bmp->pixels = (int*) malloc(pixelSize * sizeof(int));
    

    // Check the magic: it should start with "BM"
    if (file[0] != 'B' ||  file[1] != 'M'){
        printf("There is an error");
        return -1;
    }

    if (*((short*)(file +28)) != 24){
        printf("There is an error");
        return -1;
    }

     if (*((int*)(file + 30)) != 0){
        printf("There is an error");
        return -1;
    }


   
    int stride = bmp_file_stride(bmp);

    for(int i =0; i< h; ++i){
    int a = 0;
        for ( int j =0; j<w; ++j){
         int pixel;
         int b = *(file + offset++);
         int g = *(file + offset++);
         int r = *(file + offset++);
         rgb_to_pixel(&pixel, r, g, b);
         bmp->pixels[pixelSize - (w*(i + 1)) + a] = pixel;
         a++;
        }
        
        offset += stride % 3;
     }
        
  return 0;

    // Default implementation: just returns 0.
    
}

void write_bitmap(void *bmp_file, struct bitmap *bmp)
{
    
    // Cast bmp_file to a byte * so we can access it
    // byte by byte.
    byte *file = (byte *) bmp_file;

    // Fill in the rest :)
    file[0]= 'B';
    file[1] ='M';
    
    
    int  w = bmp->width;
    int  h = bmp->height;
    
    
     *((int*) (file  + 18)) = w;
     *((int *)(file + 22))= h;

    int stride = bmp_file_stride(bmp);
    int ImageSize = stride * h;
    
    *((int*)(file + 34)) = ImageSize;
    *((int*)(file +2)) = bmp_file_size(bmp);

    int offset =54;
    *((int *)(file + 10))= offset;
   
    int headerSize = 40;
    *((int *)(file + 14)) = headerSize;

    short colordepth = 24;
     *((short *)(file + 28)) = colordepth;

    short colorplanes = 1;
     *((short *)(file + 26)) = colorplanes;

    int r, g, b;
    int index = 0;

    
    for(int i = h-1; i>=0; i--){
        for ( int j =0; j<w; ++j){
         int pixel = bmp->pixels[i*w +j] ;
         pixel_to_rgb(pixel, &r, &g, &b);
         file[ offset + index++] =b;
         file[ offset + index++] =g;
         file[ offset + index++] =r;

         }

        offset += stride % 3;

     }
  }

void rgb_to_pixel(int *p, int r, int g, int b)
{
    // Pack r, g, and b into an int value and save
    // into what p points to

    *p = (r&0xff)<<16| (g&0xff)<< 8|(b&0xff);

}

void pixel_to_rgb(int p, int *r, int *g, int *b)
{
    // Separate the pixel p into its components
    // and save in the pointers
    *r = p >>16;
    *g = (p&0x00ff00)>>8;
    *b = (p&0xff);

}

void bitmap_to_grayscale(struct bitmap *bmp){
    int  r,g,b;
    int w  = bmp->width;
    int h = bmp->height;
    int pixel;

    for(int i =0; i<h; ++i){
        for (int j =0; j<w; ++j){
            pixel = bmp->pixels[i*w +j] ;
            pixel_to_rgb(pixel,&r,&g,&b);
            int grayscale  = ( r + g + b)/3;

            rgb_to_pixel(&pixel,grayscale,grayscale,grayscale);
            bmp->pixels[i*w+j] = pixel;
        }
    }
}

void bitmap_posterize(struct bitmap *bmp){
   int  r,  g, b;
   int w  = bmp->width;
   int h = bmp->height;
   int pixel;
   int *colors[3] = {&r, &g, &b};

   for(int i =0; i<h; ++i){
      for (int j =0; j<w; ++j){
         pixel = bmp->pixels[i*w +j] ;
         pixel_to_rgb(pixel,&r,&g,&b);
         for (int a = 0; a<3; a++){
            if (*colors[a]<32){
                *colors[a] =0;
                }
           else if (*colors[a]<= 95){
                *colors[a] = 64;
                }
           else if (*colors[a]<= 159){
                *colors[a] =128;
                }
           else if (*colors[a]<=223){
                *colors[a] = 192;
                }
           else if (*colors[a]>=244){
                *colors[a] =255;
                }
          rgb_to_pixel(&pixel,r,g,b);
          bmp->pixels[i*w+j] =pixel;
          }
      }
    }
   }



void bitmap_mirror(struct bitmap *bmp){
    int new_width = bmp->width * 2;
    int h = bmp->height;
    int w = bmp->width;
    int pixelSize = h * new_width;
    int *new_pixels = (int*) malloc(pixelSize * sizeof(int));
    int pixel;

    for(int i =0; i<h; ++i){
         for (int j =0; j<w; ++j){
            pixel = bmp->pixels[i*w +j] ;
            new_pixels[i*new_width +j] = pixel;
            new_pixels[i*new_width+ (new_width -j-1)]= pixel;


        }
    }
    free(bmp->pixels);
    bmp->width = new_width;
    bmp->pixels = new_pixels;
}

void bitmap_squash(struct bitmap *bmp){
    int r,g ,b;
    int r1, r2, g1, g2, b1,b2;
    int pixel, pixel2;

    int new_width = bmp->width/2;
    int h = bmp->height;
    int w = bmp->width;
    int pixelSize = h * new_width;
    int *new_pixels = (int*) malloc(pixelSize * sizeof(int));

    for(int i =0; i<h; ++i){
         for (int j =0; j<w; ++j){
            pixel = bmp->pixels[i*w +(2*j)] ;
            pixel2 = bmp->pixels[i*w+((2*j)+1)];
            pixel_to_rgb(pixel,&r1, &g1, &b1);
            pixel_to_rgb(pixel2, &r2,&g2 , &b2);

            r = (r1+r2)/2;
            g = (g1+g2)/2;
            b = (b1+b2)/2;

           rgb_to_pixel(&pixel , r,g,b);
           new_pixels[i*new_width+j] = pixel;

        }
    }
    free(bmp->pixels);
    bmp->width = new_width;
    bmp->pixels = new_pixels;
    }
    

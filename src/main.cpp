#define VITASDK

#include <psp2/sysmodule.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/display.h>
//#include <time.h>
//#include <ctime>
#include <chrono>
#include <ctime>

#include <psp2/libssl.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/net/http.h>

#include <psp2/io/fcntl.h>

#include <stdio.h>
#include <malloc.h> 
#include <string>
#include <vector>
#include <cstring>

#include <sys/time.h>

#include <vita2d.h>
#include "header/Logger.hpp"
#include "giflib/gif_lib.h"



extern "C" {
	int _newlib_heap_size_user = 128 * 1024 * 1024;
}

vita2d_texture * myGifVita2D;
GifFileType * myGif;
float gifX;
float gifY;
float gifScaleX;
float gifScaleY;

void LoadStaticGif(){
	int ErrorCode = 0;
	myGif = DGifOpenFileName("ux0:data/gifs/banana.gif", &ErrorCode);
	if(ErrorCode == 0){
		//debugNetPrintf(DEBUG , "GIF Opened! \r\n");
	}else{
		//debugNetPrintf(ERROR , "Error Open GIF : %d \r\n" , ErrorCode);
		return;
	}
	if(DGifSlurp(myGif) == GIF_OK){
		//debugNetPrintf(DEBUG , "GIF Slurped! \r\n");
	}else{
		//debugNetPrintf(DEBUG , "Failed Gif Slurp \r\n");
		return;
	}
	
	myGifVita2D = vita2d_create_empty_texture( myGif->SWidth,myGif->SHeight);
	unsigned int stride = vita2d_texture_get_stride(myGifVita2D);
	unsigned char *texp = vita2d_texture_get_datap(myGifVita2D);
	unsigned char *gifp = myGif->SavedImages[0].RasterBits;
	GifColorType * gifPalette = myGif->SColorMap->Colors;
	
	
	unsigned char * gifRGBA8 = new unsigned char[myGif->SWidth * myGif->SHeight * 4];
	int j = 0;
	for(unsigned int i = 0 ; i < myGif->SWidth * myGif->SHeight * 4 ; i+=4){
		
		gifRGBA8[i] = gifPalette[  gifp[j]].Red;
		gifRGBA8[i+1] = gifPalette[gifp[j]].Green;
		gifRGBA8[i+2] = gifPalette[gifp[j]].Blue;
		gifRGBA8[i+3] = 255;
		j++;
	}
	//memcpy(texp, gifRGBA8,myGif->SWidth * myGif->SHeight *4  );
	
	for (unsigned int x = 0; x < myGif->SHeight; x++) {
		memcpy(texp, gifRGBA8, myGif->SWidth * 4);
		gifRGBA8 += myGif->SWidth * 4;
		texp += stride;
	}
	
}

void DrawGif(){
	vita2d_start_drawing();
	vita2d_clear_screen();
	
	if(myGifVita2D != NULL){
		vita2d_draw_texture_scale(myGifVita2D , gifX , gifY , gifScaleX , gifScaleY);
	}
	
	vita2d_end_drawing();
	vita2d_swap_buffers();
}


struct GifThreadHelper{
	vita2d_texture * myGifVita2D;
	GifFileType * myGif;
};


#define UNSIGNED_LITTLE_ENDIAN(lo, hi)	((lo) | ((hi) << 8))
#define GIF_WAIT_TIME_BASE_MULTIPLIER 10000
void LoadAnimationGif(){
	//Logger::Info(" In LoadAnimationGif \n\n");
	int ErrorCode = 0;
	myGif = DGifOpenFileName("ux0:data/gifs/banana.gif", &ErrorCode);
	if(ErrorCode == 0){
		//debugNetPrintf(DEBUG , "GIF Opened! \r\n");
	}else{
		//debugNetPrintf(ERROR , "Error Open GIF : %d \r\n" , ErrorCode);
		return;
	}
	if(DGifSlurp(myGif) == GIF_OK){
		//debugNetPrintf(DEBUG , "GIF Slurped! \r\n");
	}else{
		//debugNetPrintf(DEBUG , "Failed Gif Slurp \r\n");
		return;
	}
	
	
	//debugNetPrintf(DEBUG , "Dimensions width / height :   %d  x  %d \r\n" , myGif->SWidth , myGif->SHeight );
	
	gifScaleX = 1;
	gifScaleY = 1;
	
	float newSizeX = myGif->SWidth;
	float newSizeY = myGif->SHeight;
	gifScaleY = 544 / newSizeY;
	gifScaleX = 960 / newSizeX;
	
	gifScaleX = (gifScaleX < gifScaleY) ? gifScaleX : gifScaleY;
	gifScaleY = gifScaleX;
	newSizeX = newSizeX  * gifScaleX;
	newSizeY = newSizeY  * gifScaleY;

	gifX = (960 / 2)  -  (newSizeX / 2);
	gifY = (544 / 2)  -  (newSizeY / 2);
	//debugNetPrintf(DEBUG , "scaleX : %f \r\n" , gifScaleX );
	//debugNetPrintf(DEBUG , "scaleY : %f \r\n" , gifScaleY );
	
	//debugNetPrintf(DEBUG , "Image count : %d \r\n" , myGif->ImageCount );
	//debugNetPrintf(DEBUG , "Creating texture \r\n");
	myGifVita2D = vita2d_create_empty_texture( myGif->SWidth,myGif->SHeight);
	//debugNetPrintf(DEBUG , "Getting stride \r\n");
	unsigned int stride = vita2d_texture_get_stride(myGifVita2D);
	//debugNetPrintf(DEBUG , "Stride is : %u \r\n" , stride);
	//debugNetPrintf(DEBUG , "Getting texture datap \r\n");
	unsigned char *texp = vita2d_texture_get_datap(myGifVita2D);
	unsigned char *gifp = NULL ;
	GifColorType * gifPalette;
	bool hasGlobalPalette = false;
	if(myGif->SColorMap == NULL){
		hasGlobalPalette = false;
		//debugNetPrintf(DEBUG , "Has no global palette SColorMap IS NULL !!! \r\n");
	}else{
		hasGlobalPalette = true;
		//debugNetPrintf(DEBUG , "Getting gifPalette from myGif->SColorMap->Colors \r\n");
		gifPalette = myGif->SColorMap->Colors;
	}
	unsigned int delay = 1;
	//debugNetPrintf(DEBUG , "Creating new gifRGBA8 \r\n");
	unsigned char * gifRGBA8 = new unsigned char[myGif->SWidth * myGif->SHeight * 4];
	GraphicsControlBlock myGifGCBlock;
	//debugNetPrintf(DEBUG , "Initiating while loop \r\n");
	
	unsigned char backgroundRed = (((myGif->SBackGroundColor)&0xFF)>>0);
	unsigned char backgroundGreen = (((myGif->SBackGroundColor)&0xFF)>>8);
	unsigned char backgroundBlue = (((myGif->SBackGroundColor)&0xFF)>>16);
	// for easy sleep : 
	struct timeval t1, t2;
	double elapsedTime;
	
	
	
	
	unsigned char * gifRGBA8LastUndisposed = new unsigned char[myGif->SWidth * myGif->SHeight * 4];
	
	int CurrentDisposalMode = 0; 
	
	//struct timeval tDelay1, tDelay2;
	//double delayTimeReduction;
	//struct timespec begin;
	//struct timespec end;
	//clock_gettime( CLOCK_MONOTONIC, &begin );
	std::chrono::time_point<std::chrono::system_clock> start, end;
	int delayTimeReduction;
	start = std::chrono::system_clock::now();
	
	//gettimeofday(&tDelay1, NULL);
	while(true){
		for(int i = 0; i < myGif->ImageCount ; i++){
			//debugNetPrintf(INFO , "Current frame %d \r\n" , i);
			gettimeofday(&t1, NULL);
			unsigned char *texpLoop = texp;
			unsigned char *gifRGBA8Loop = gifRGBA8;
			DGifSavedExtensionToGCB(myGif , i , &myGifGCBlock);
			gettimeofday(&t2, NULL);
			elapsedTime = (t2.tv_usec - t1.tv_usec);
			debugNetPrintf(DEBUG , "T1 DGifSavedExtensionToGCB Elapsed time : %f!\r\n" , elapsedTime);
			//if(currentSImage.ExtensionBlocks->ByteCount == 4){
			gettimeofday(&t1, NULL);
			SavedImage currentSImage = myGif->SavedImages[i];
				gifp = currentSImage.RasterBits;
				//debugNetPrintf(INFO , "Left : %d , Top : %d , Width : %d , Height : %d\r\n" , currentSImage.ImageDesc.Left ,currentSImage.ImageDesc.Top , currentSImage.ImageDesc.Width, currentSImage.ImageDesc.Height );
				//debugNetPrintf(INFO , currentSImage.ImageDesc.Interlace ? "Interlaced Image!\r\n" : "Sequential image!\r\n" );
				
				
				if(hasGlobalPalette == false){
					gifPalette = currentSImage.ImageDesc.ColorMap->Colors;
				}else{
					if(currentSImage.ImageDesc.ColorMap != NULL){
						gifPalette = currentSImage.ImageDesc.ColorMap->Colors;
					}else{
						// using global one
					}
				}
				delay = myGifGCBlock.DelayTime;
				//delay = UNSIGNED_LITTLE_ENDIAN(currentSImage.ExtensionBlocks->Bytes[1], currentSImage.ExtensionBlocks->Bytes[2]);
				//debugNetPrintf(INFO , "Delay time : %u \r\n " , delay);
				
				CurrentDisposalMode = myGifGCBlock.DisposalMode;
			gettimeofday(&t2, NULL);
			elapsedTime = (t2.tv_usec - t1.tv_usec);
			debugNetPrintf(DEBUG , "T2 Stuff Elapsed time : %f!\r\n" , elapsedTime);
				
				
				
				
				if( CurrentDisposalMode == DISPOSE_DO_NOT || CurrentDisposalMode == DISPOSAL_UNSPECIFIED){
					gettimeofday(&t1, NULL);
					memcpy(gifRGBA8LastUndisposed , gifRGBA8 , myGif->SWidth * myGif->SHeight * 4);
					gettimeofday(&t2, NULL);
					elapsedTime = (t2.tv_usec - t1.tv_usec);
					debugNetPrintf(DEBUG , "T3 memcpy to lastundisposed Elapsed time : %f!\r\n" , elapsedTime);
				}
				
				////debugNetPrintf(INFO , "Looping for palette\r\n");
				
			gettimeofday(&t1, NULL);
				int currentPixel = 0;
				int currentPaletteIndex = 0;
				int bytesPerPixelV2D = 4;
				int rowWidth = myGif->SWidth * bytesPerPixelV2D;
				int currentRGBA8ArrayIndex = currentSImage.ImageDesc.Top * rowWidth  + currentSImage.ImageDesc.Left * bytesPerPixelV2D;
				int currentRGBA8ArrayIndexSecondary = currentSImage.ImageDesc.Top * rowWidth  + currentSImage.ImageDesc.Left * bytesPerPixelV2D;
				int TransparentColorIndex = myGifGCBlock.TransparentColor;
				if( CurrentDisposalMode == DISPOSE_DO_NOT || CurrentDisposalMode == DISPOSAL_UNSPECIFIED){
					for(int y = currentSImage.ImageDesc.Top; y < currentSImage.ImageDesc.Top + currentSImage.ImageDesc.Height ; y ++){
						currentRGBA8ArrayIndexSecondary = currentRGBA8ArrayIndex;
						for(int x = currentSImage.ImageDesc.Left; x < currentSImage.ImageDesc.Left + currentSImage.ImageDesc.Width ; x++){
							//currentRGBA8ArrayIndex = y * rowWidth + x * 4;
							currentPaletteIndex = gifp[currentPixel];
							if(TransparentColorIndex == gifp[currentPixel]){
								
							}else{
								gifRGBA8Loop[currentRGBA8ArrayIndexSecondary + 0] = gifPalette[ currentPaletteIndex ].Red;
								gifRGBA8Loop[currentRGBA8ArrayIndexSecondary + 1] = gifPalette[ currentPaletteIndex ].Green;
								gifRGBA8Loop[currentRGBA8ArrayIndexSecondary + 2] = gifPalette[ currentPaletteIndex ].Blue;
								gifRGBA8Loop[currentRGBA8ArrayIndexSecondary + 3] = 255;
							}
							currentPixel++;
							currentRGBA8ArrayIndexSecondary += 4;
							//currentRGBA8ArrayIndex +=  bytesPerPixelV2D;
						}
						currentRGBA8ArrayIndex += rowWidth;
						//currentRGBA8ArrayIndex = y * rowWidth  + currentSImage.ImageDesc.Left * bytesPerPixelV2D;
					}
					
				}else{
							
					for(int y = currentSImage.ImageDesc.Top; y < currentSImage.ImageDesc.Top + currentSImage.ImageDesc.Height ; y ++){
						currentRGBA8ArrayIndexSecondary = currentRGBA8ArrayIndex;
						for(int x = currentSImage.ImageDesc.Left; x < currentSImage.ImageDesc.Left + currentSImage.ImageDesc.Width ; x++){
							//currentRGBA8ArrayIndex = y * rowWidth + x * 4;
							currentPaletteIndex = gifp[currentPixel];
							if(myGifGCBlock.TransparentColor == gifp[currentPixel]){
								gifRGBA8Loop[currentRGBA8ArrayIndexSecondary + 3] = 0;
							}else{
								gifRGBA8Loop[currentRGBA8ArrayIndexSecondary + 0] = gifPalette[ currentPaletteIndex ].Red;
								gifRGBA8Loop[currentRGBA8ArrayIndexSecondary + 1] = gifPalette[ currentPaletteIndex ].Green;
								gifRGBA8Loop[currentRGBA8ArrayIndexSecondary + 2] = gifPalette[ currentPaletteIndex ].Blue;
								gifRGBA8Loop[currentRGBA8ArrayIndexSecondary + 3] = 255;
							}
							currentPixel++;
							currentRGBA8ArrayIndexSecondary += 4;
							//currentRGBA8ArrayIndex +=  bytesPerPixelV2D;
						}
						currentRGBA8ArrayIndex += rowWidth;
						//currentRGBA8ArrayIndex = y * rowWidth  + currentSImage.ImageDesc.Left * bytesPerPixelV2D;
					}
				}
				
			gettimeofday(&t2, NULL);
			elapsedTime = (t2.tv_usec - t1.tv_usec);
			debugNetPrintf(DEBUG , "T4 Palette to color array Elapsed time : %f!\r\n" , elapsedTime);
				
				
				//int j = 0;
				//for(unsigned int i = 0 ; i < myGif->SWidth * myGif->SHeight * 4 ; i+=4){
				//	// this is not correct yet : 
				//	
				//	if(myGifGCBlock.TransparentColor == gifp[j]){
				//		//gifRGBA8Loop[i+3] = 0;
				//	}else{
				//		gifRGBA8Loop[i] = gifPalette[  gifp[j]].Red;
				//		gifRGBA8Loop[i+1] = gifPalette[gifp[j]].Green;
				//		gifRGBA8Loop[i+2] = gifPalette[gifp[j]].Blue;
				//		gifRGBA8Loop[i+3] = 255;
				//	}
				//	
				//	
				//	
				//	j++;
				//}
				
				
				
			gettimeofday(&t1, NULL);
				////debugNetPrintf(INFO , "Looping memcpy texture\r\n");
				for (unsigned int x = 0; x < myGif->SHeight; x++) {
					memcpy(texpLoop, gifRGBA8Loop, myGif->SWidth * 4);
					gifRGBA8Loop += myGif->SWidth * 4;
					texpLoop += stride;
				}
			gettimeofday(&t2, NULL);
			elapsedTime = (t2.tv_usec - t1.tv_usec);
			debugNetPrintf(DEBUG , "T5 Memcpy to vita2d texture Elapsed time : %f!\r\n" , elapsedTime);
				
				//delay = GIF_WAIT_TIME_BASE_MULTIPLIER * delay - (unsigned int)elapsedTime;
				delay = GIF_WAIT_TIME_BASE_MULTIPLIER * delay;
				//if( delay > 10000000){
				//	
				//}else{
				//	if(delay > 1000000){
				//		delay = myGifGCBlock.DelayTime * 10000;
				//	}
				//	//debugNetPrintf(INFO , "Waiting : %u microseconds\r\n " , (delay));
				//	sceKernelDelayThread(delay);
				//}
				
				
				
				//gettimeofday(&tDelay2, NULL);
				//
				//delayTimeReduction = (tDelay2.tv_usec - tDelay1.tv_usec);
				//debugNetPrintf(DEBUG , "TIME TO DISPLAY NOW : %f \r\n" , delayTimeReduction);
				//if(delayTimeReduction > delay){
				//	
				//}else{
				//	sceKernelDelayThread(delay - delayTimeReduction );
				//}
				
				//clock_gettime(CLOCK_MONOTONIC, &end );
				//delayTimeReduction = double(end.tv_sec - begin.tv_sec) * 1000000  + (end.tv_nsec - begin.tv_nsec)/1000.0
				
				end = std::chrono::system_clock::now();
				delayTimeReduction = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
				debugNetPrintf(DEBUG , "TIME TO DISPLAY NOW : %f \r\n" , delay - delayTimeReduction);
				if(delayTimeReduction >= delay){
					
				}else{
					sceKernelDelayThread(delay - delayTimeReduction );
				}
				
				// dispose time
				// this is not correct yet : 
			gettimeofday(&t1, NULL);
				switch(CurrentDisposalMode){
					case DISPOSAL_UNSPECIFIED :
					{
						//debugNetPrintf(INFO , "Disposal Mode : DISPOSAL_UNSPECIFIED \r\n");
						//memcpy( gifRGBA8, gifRGBA8LastUndisposed  , myGif->SWidth * myGif->SHeight * 4);
					}
					break;
					case DISPOSE_DO_NOT :
					{
						//debugNetPrintf(INFO , "Disposal Mode : DISPOSE_DO_NOT \r\n");
						//memcpy( gifRGBA8, gifRGBA8LastUndisposed  , myGif->SWidth * myGif->SHeight * 4);
					}
					break;
					case DISPOSE_BACKGROUND :
					{
						//debugNetPrintf(INFO , "Disposal Mode : DISPOSE_BACKGROUND \r\n");
						currentPixel = 0;
						currentRGBA8ArrayIndex = currentSImage.ImageDesc.Top * rowWidth  + currentSImage.ImageDesc.Left * bytesPerPixelV2D;
						currentRGBA8ArrayIndexSecondary = currentSImage.ImageDesc.Top * rowWidth  + currentSImage.ImageDesc.Left * bytesPerPixelV2D;
						for(int y = currentSImage.ImageDesc.Top; y < currentSImage.ImageDesc.Top + currentSImage.ImageDesc.Height ; y ++){
							currentRGBA8ArrayIndexSecondary = currentRGBA8ArrayIndex;
							for(int x = currentSImage.ImageDesc.Left; x < currentSImage.ImageDesc.Left + currentSImage.ImageDesc.Width ; x++){
								//currentRGBA8ArrayIndex = y * myGif->SWidth * 4 + x * 4;
								if(myGifGCBlock.TransparentColor == gifp[currentPixel]){
									
									gifRGBA8[currentRGBA8ArrayIndex + 3] = 0;
									
								}else{
									gifRGBA8[currentRGBA8ArrayIndex + 0] = backgroundRed;
									gifRGBA8[currentRGBA8ArrayIndex + 1] = backgroundGreen;
									gifRGBA8[currentRGBA8ArrayIndex + 2] = backgroundBlue;
									gifRGBA8[currentRGBA8ArrayIndex + 3] = 0;
								}
								currentPixel++;
								currentRGBA8ArrayIndexSecondary += 4;
								//currentRGBA8ArrayIndex += bytesPerPixelV2D;
							}
							currentRGBA8ArrayIndex += rowWidth;
							//currentRGBA8ArrayIndex = y * rowWidth  + currentSImage.ImageDesc.Left * bytesPerPixelV2D;
						}
						
						
					}
					break;
					case DISPOSE_PREVIOUS :
					{
						//debugNetPrintf(INFO , "Disposal Mode : DISPOSE_PREVIOUS \r\n");
						memcpy( gifRGBA8, gifRGBA8LastUndisposed  , myGif->SWidth * myGif->SHeight * 4);
					}
					break;
				}
			gettimeofday(&t2, NULL);
			elapsedTime = (t2.tv_usec - t1.tv_usec);
			debugNetPrintf(DEBUG , "T6 Switch for Disposal mode Elapsed time : %f!\r\n" , elapsedTime);
			//gettimeofday(&tDelay1, NULL);
			//clock_gettime( CLOCK_MONOTONIC, &begin );
			start = std::chrono::system_clock::now();
			//}
		}
	}
	
	delete[] gifRGBA8LastUndisposed;
	delete[] gifRGBA8;
	
}
static int GifAnimationThread(unsigned int args, void* argp){
	
	//ThreadGifHelper * thelper = (ThreadGifHelper*)argp;
	////debugNetPrintf(DEBUG, "got threadgifhelper\r\n" );
	//Logger::Info(" Starting load animation \n\n");
	LoadAnimationGif();
	
}

int main(int argc, char *argv[]) {
	Logger::Setup();
	Logger::Info(" ====   gifs  v.0102   ==== \n\n");
	//debugNetPrintf(INFO , "1 \r\n");
	
	vita2d_init();
	vita2d_set_clear_color(RGBA8(000 , 0x00 , 0x44 , 0xFF));
	
	if(false){
		LoadStaticGif();
	}else{
		SceUID threadID = sceKernelCreateThread ("gifthread", &GifAnimationThread, 0x40, 1024*1024, 0 , 0 , NULL);
		//Logger::Info(" Starting thread \n\n");
		sceKernelStartThread( threadID , 0, NULL );	
	}
	
	gifX = 0;
	gifY = 0;
	
	while(true){
		//gifX++;
		//if(gifX > 800){
		//	gifX = 0;
		//}
		//gifY++;
		//if(gifY > 400){
		//	gifY = 0;
		//}
		DrawGif();
	}
	//debugNetPrintf(DEBUG , "This app will close in 10 seconds!\r\n");
	sceKernelDelayThread(10*1000*1000);
	sceKernelExitProcess(0);
	return 0;
}

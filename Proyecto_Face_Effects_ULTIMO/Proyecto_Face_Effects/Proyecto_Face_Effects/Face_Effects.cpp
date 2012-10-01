#include <cxtypes.h>
#include <cxcore.h>
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*Definicion de Funciones*/

IplImage* FunctionSAL(IplImage* img1);
IplImage* FunctionPIMIENTA(IplImage* img1);
IplImage* FunctionSALYPIMIENTA(IplImage* img1);
IplImage* agregarRuido(IplImage *imag);
IplImage *filtro_medio(IplImage *imag);
IplImage *filtro_maximo(IplImage *imag);
IplImage *filtro_minimo(IplImage *imag);
IplImage* eliminarRuido(IplImage *imag);
void animacionUno(CvRect* r, IplImage* img);
void animacionDos(CvRect* r, IplImage* img);
void presentacioFaceEfect();
void menuFace_Efect();
void manejarOpciones(int op);


//FUNCIONES BRILLO CONTRASTE
void editorHist(IplImage *img);

IplImage* Stretch(IplImage * img_in, double StretchFactor);
IplImage* Shrink(IplImage * img_in, double ShrinkFactor);
IplImage* Desplazamiento(IplImage * imag, int offset);

void actualizarBrillo(int val);
void crearHistogramas(IplImage * img);
void graficarHistogramas();

// Create memory for calculations
static CvMemStorage* storage0 = 0;

// Create a new Haar classifier
static CvHaarClassifierCascade* cascade = 0;

// Function prototype for detecting and drawing an object from an image
void detect_and_draw( IplImage* image );

// Create a string that contains the cascade name
const char* cascade_name ="haarcascade_frontalface_alt.xml";

//Variables que controlan la activacion de las animaciones
int animacion1_opc=0;
int animacion2_opc=0;
int tipo_ruido=0;

//VARIABLES GLOBALES
IplImage *img_dst;
IplImage *r,*g,*b,*r2,*g2,*b2;
CvHistogram* histR,*histG,*histB;//histograma resultante
IplImage* imgHistR, *imgHistG, *imgHistB;

int brightness=100;
int contrast=10;



/*
 * Nombre:
 * =======
 *              int main( int argc, char** argv )
 * Descripcion:
 * ============
 *              main principal del programa, en el cual se carga la imagen de
 *              inicio obtenida de la camara, para hacer los respectivos procesamiento
 *              que se requiere para hacer la animacion
 *              
*/
int main( int argc, char** argv )
{
	int num;
	menuFace_Efect();
	presentacioFaceEfect();
    return 0;
}

void presentacioFaceEfect(){
// Structure for getting video from camera or avi
    CvCapture* capture = 0;  
    // Images to capture the frame from video or camera or from file
    IplImage *frame, *frame_copy = 0;

    // Load the HaarClassifierCascade
    cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );

    // Check whether the cascade has loaded successfully. Else report and error and quit
    if( !cascade )
    {
        fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
        //return -1;
    }

    // Allocate the memory storage
    storage0 = cvCreateMemStorage(0);

	// Create a new named window with title: result
    cvNamedWindow( "result", 1 );
	//cvNamedWindow( "ruido", 1 );

    // Find if the capture is loaded successfully or not.
    // If loaded succesfully, then:
    int i=0;
    capture = cvCaptureFromCAM(0);
    if ( !capture ) {
        capture = cvCaptureFromCAM(1);
        if ( !capture ) {
            fprintf( stderr, "Cannot open initialize webcam!\n" );
            //return 1;
        }
    }
	

	int cont=0;
    if( capture )
    {
        for(;;i++)
        {
            // Capture the frame and load it in IplImage
            if( !cvGrabFrame( capture ))
                break;
            frame = cvRetrieveFrame( capture );

            // If the frame does not exist, quit the loop
            if( !frame )
                break;

            // Allocate framecopy as the same size of the frame
            if( !frame_copy )
                frame_copy = cvCreateImage( cvSize(frame->width,frame->height), IPL_DEPTH_8U, frame->nChannels );

            // Check the origin of image. If top left, copy the image frame to frame_copy.
            if( frame->origin == IPL_ORIGIN_TL )
                cvCopy( frame, frame_copy, 0 );
            // Else flip and copy the image
            else
                cvFlip( frame, frame_copy, 0 );
				
			//frame_copy = FunctionSALYPIMIENTA(frame_copy);
		    //frame_copy = filtro_medio(frame_copy);
			if(cont>=1){
				//printf("entro aqui a poner ruido");
				frame_copy=agregarRuido(frame_copy);
				//cvShowImage("ruido", frame_copy);
			}
            // Call the function to detect and draw the face
            detect_and_draw( frame_copy );
			cont++;
			if(cont==1) printf("\n\t\t\t=====PRESIONE C PARA CAPTURAR UNA IMAGEN=====");
			 //Wait for a while before proceeding to the next frame
            char c =  cvWaitKey( 10 );
			
           if (c== 27)  exit(0);
		   if(c == 'c' || c == 'C'){
						cvNamedWindow( "Captura", 1 );
						cvShowImage( "Captura", frame_copy );
						cvWaitKey( 0 );
						frame_copy= eliminarRuido(frame_copy);
						printf("\n\t\t\t\t=====SE ELIMINO EL RUIDO=====");
						cvShowImage( "Captura", frame_copy );
						cvWaitKey( 0 );
						editorHist(frame_copy);
						cvWaitKey( 0 );
		   }

		}	
        // Release the images, and capture memory
        cvReleaseImage( &frame_copy );
        cvReleaseCapture( &capture );
    }

    // Destroy the window previously created with filename: "result"
    cvDestroyWindow("result");

}

/*
 * Nombre:
 * =======
 *               IplImage* crearMascara(IplImage* orig)
 * Descripcion:
 * ============
 *              Funcion que sirve para obtener una mascara de la imagen enviada como parametro
 *              y la retorna para ser utilizada en los demas procesos
*/

IplImage* crearMascara(IplImage* orig){
	IplImage *gris = cvCreateImage(cvSize(orig->width,orig->height),orig->depth,1);
	IplImage *bin = cvCreateImage(cvSize(orig->width,orig->height),orig->depth,orig->nChannels);
	cvCvtColor(orig,gris,CV_BGR2GRAY);
	cvThreshold(gris,gris,230,255,CV_THRESH_BINARY);
	cvNot(gris,gris);
	cvErode(gris,gris,NULL,1);
	cvCvtColor(gris,bin,CV_GRAY2BGR);
	return bin;

}

/*
 * Nombre:
 * =======
 *               void fusionImagenes(IplImage* frame, IplImage* objeto,CvRect* r, double escalamiento, double despX,double despY)
 * Descripcion:
 * ============
 *              Funcion que sirve para fusionar las imagenes que se va a poner como animacion
 *              en el rostro de la persona que esta frente a la camara, recibe como parametro
 *              es escalamiento que debe tener, y la posicion correspondiente para dicho efecto.
*/

void fusionImagenes(IplImage* frame, IplImage* objeto,CvRect* r, double escalamiento, double despX,double despY){
	
	IplImage *imgCopy,*imgObj,*imgObj2,*imgMask,*imgMask2,*objResized,*maskResized;
	
	IplImage* objetoMask = crearMascara(objeto);
	imgCopy = cvCloneImage(frame);//Se clona la Imagen que captura la camara
	imgObj = cvCreateImage(cvSize(frame->width,frame->height),frame->depth,frame->nChannels);//imagen para pegar el objeto
	imgObj2 = cvCreateImage(cvSize(frame->width,frame->height),frame->depth,frame->nChannels);//imagen anterior pero  trasladada
	
	imgMask = cvCreateImage(cvSize(frame->width,frame->height),frame->depth,frame->nChannels);//imagen para pegar la Mascara
	imgMask2 = cvCreateImage(cvSize(frame->width,frame->height),frame->depth,frame->nChannels);//imagen anterior pero  trasladada

	int newW = (int)(escalamiento*objeto->width*r->width);//tamaño horizontal nuevo para el objeto, tiene que ser proporcional al de r, por eso lo multiplico
	int newH = (int)(escalamiento*objeto->height*r->height);//tamaño vertical nuevo para el objeto..

	if(newW > frame->width) newW = frame->width;	// valida que el nuevo tamaño horizontal del objeto no sobrepase el del frame
	if(newH > frame->height) newH = frame->height;  // valida que el nuevo tamaño vertical del objeto no sobrepase el del frame

	objResized = cvCreateImage(cvSize(newW,newH),objeto->depth,objeto->nChannels);//Imagen con el nuevo tamaño
	cvResize(objeto,objResized,1);//crea las gafas con el nuevo tamaño
	
	maskResized = cvCreateImage(cvSize(newW,newH),objetoMask->depth,objetoMask->nChannels);//Mascara con el nuevo tamaño
	cvResize(objetoMask,maskResized,1);//crea la mascara con el nuevo tamaño
	
	cvSetImageROI(imgObj, cvRect(0,0,objResized->width,objResized->height));//crea un roi en el origen con el tamaño del objeto
	cvCopy(objResized,imgObj);//copia el objeto
	cvResetImageROI(imgObj);//quita el roi
	
	cvSetImageROI(imgMask, cvRect(0,0,maskResized->width,maskResized->height));
	cvCopy(maskResized,imgMask);
	cvResetImageROI(imgMask);
	
	int dx = r->x+despX;//La posicion x del rectangulo que detecta la cara+desplazamiento basado en el ancho del rectangulo
	int dy = r->y+despY;
		
	//Matriz de transformacion, solo tomamos en cuenta dx,dy para la traslacion
	CvMat *M = cvCreateMat( 2, 3, CV_32FC1);
	cvmSet(M,0,0,1); // asignamos valor 1 al elemento (0,0)
	cvmSet(M,0,1,0); // asignamos valor 0 al elemento (0,1)
	cvmSet(M,1,0,0); // asignamos valor 0 al elemento (1,0)
	cvmSet(M,1,1,1); // asignamos valor 1 al elemento (1,1)
	cvmSet(M,0,2,dx); // el cuarto número indica los píxeles que se recorren en el eje x
	cvmSet(M,1,2,dy);
	
	cvWarpAffine (imgObj, imgObj2, M, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS,cvScalarAll(0));// aplicamos la transformación para obtener la imagen trasladada
	cvWarpAffine (imgMask,imgMask2, M, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS,cvScalarAll(0));//lo mismo pero para la mascara
	
	 
	  
	cvAnd(imgObj2,imgMask2,imgObj2,0);//Se recorta el objeto usando la mascara
	cvNot(imgMask2,imgMask2);			//Se crea la mascara inversa
	cvAnd(imgCopy,imgMask2,imgCopy,0);	//Se usa la macara inversa para quitar la parte en donde ira el objeto
	cvOr(imgCopy,imgObj2,frame,0);		// Se unen las dos imagenes con OR

	//Libero memoria de cada una de las imagenes		
	cvReleaseImage(&imgCopy);
	cvReleaseImage(&imgObj);
	cvReleaseImage(&imgObj2);
	cvReleaseImage(&objetoMask);
	cvReleaseImage(&imgMask);
	cvReleaseImage(&imgMask2);
	cvReleaseImage(&objResized);
	cvReleaseImage(&maskResized);
}



/*
 * Nombre:
 * =======
 *              void detect_and_draw( IplImage* img )
 * Descripcion:
 * ============
 *              Funcion que detecta el rostro de la personas para poder 
 *              calcular las diferentes areas donde se le va aplicar una
 *              insercion de una imagen para dar la animacion que se desea dar.
 *              
*/

// Function to detect and draw any faces that is present in an image
void detect_and_draw( IplImage* img )
{
    int i;
    // Clear the memory storage which was used before
    cvClearMemStorage( storage0 );
   
    // Find whether the cascade is loaded, to find the faces. If yes, then:
    if( cascade )
    {
        // There can be more than one face in an image. So create a growable sequence of faces.
        // Detect the objects and store them in the sequence
        CvSeq* faces = cvHaarDetectObjects( img, cascade, storage0, 1.1, 2, CV_HAAR_DO_CANNY_PRUNING, cvSize(40, 40) );

        // Loop the number of faces found.
        for( i = 0; i < (faces ? faces->total : 0); i++ )
        {
           // Create a new rectangle for drawing the face
            CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
			char c= cvWaitKey(33);

			switch(c){
               case 27:
                   exit(0);
                   break;
               case '1':
				   animacion1_opc = !animacion1_opc;
				   break;				   
               case '2':
				   animacion2_opc = !animacion2_opc;
				   break;
			}

			if(animacion1_opc) animacionUno(r,img);
			if(animacion2_opc) animacionDos(r,img);
        }
    }
    // Show the image in the window named "result"
    cvShowImage( "result", img );
}


void animacionUno(CvRect* r, IplImage* img)
{
	//Carga todas la imagenes de cada pieza a insertar para la animacion

	IplImage *gafas = cvLoadImage("gafas3.jpg", 1);//Objeto gafas				
	IplImage *peluca = cvLoadImage("peluca.jpg", 1);//Objeto peluca
	IplImage *bigote = cvLoadImage("bigote.jpg", 1);//Objeto bigote
	IplImage *barba = cvLoadImage("barba2.jpg", 1);//Objeto barba
	
	//fusiono las imagenes es decir la real con las de la amimacion
	fusionImagenes(img,peluca,r,0.00112,r->width/12,-r->height/2); //para la peluca	
	fusionImagenes(img,gafas,r,0.0022,r->width/12,r->height/4); //para la gafas
	fusionImagenes(img,bigote,r,0.0049,r->width/10,r->height/2.7); //para el bigote
	fusionImagenes(img,barba,r,0.0035,r->width/50,r->height/1.2); //para la barba
			
	//libero la memoria de cada variable
	cvReleaseImage(&peluca);
	cvReleaseImage(&gafas);
	cvReleaseImage(&bigote);
	cvReleaseImage(&barba);
}

void animacionDos(CvRect* r, IplImage* img){
	//cargo las diferentes imagenes
	IplImage *peluca = cvLoadImage("peluca3.jpg", 1);//Objeto gafas
	IplImage *bigote = cvLoadImage("bigotes.jpg", 1);//Objeto gafas
	//fusiono la imagenes
	fusionImagenes(img,bigote,r,0.0022,r->width/12,r->height/8);
	fusionImagenes(img,peluca,r,0.0053,-r->width/2,-r->height);
	
	//liberar memoria
	cvReleaseImage(&peluca);
	cvReleaseImage(&bigote);

}

/*************************************************************/
/*			             FunctionSAl			             */
/*************************************************************/
//Esta funcion retorna IplImage

IplImage* FunctionSAL(IplImage* img1)
{
	int i,par,a,b;
	par = cvRound(img1->height * img1->width * 0.01);
	for(i=0 ; i< par ; i++)
	{	
		a=rand()%img1->height;
		b=rand()%img1->width;
		cvSet2D(img1,a,b,cvScalar(255.0,0,0,0));
	}
	return( img1 );
}
/*************************************************************/
/*    			      FunctionPIMIENTA					     */
/*************************************************************/

IplImage* FunctionPIMIENTA(IplImage* img1)
{	
	int i,par,a,b;
	IplImage* cha1,*cha2,*cha3;
	par = cvRound(img1->height * img1->width * 0.01);
	for(i=0 ; i< par ; i++)
	{	a=rand()%img1->height;
		b=rand()%img1->width;
		cvSet2D(img1,a,b,cvScalar(0.0,0,0,0));
	}
	return( img1 );
}
/*************************************************************/
/*			          FunctionSAlYPIMIENTA	     		     */
/*************************************************************/

IplImage* FunctionSALYPIMIENTA(IplImage* img1)
{	
	FunctionSAL(img1);
	FunctionPIMIENTA(img1);
	return( img1 );
}



/*************************************************************/
/*			          Function Agregar Ruido	             */
/*************************************************************/

IplImage* agregarRuido(IplImage *imag){
	tipo_ruido = 3;//= 1 + rand()%(4);
	//printf("el tipo de ruido es %d :",tipo_ruido);
	switch(tipo_ruido){
               case 1:
				   imag = FunctionSAL(imag);
				   break;				   
               case 2:
				   imag = FunctionPIMIENTA(imag);
				   break;
			   case 3:
				   imag = FunctionSALYPIMIENTA(imag);
			   break;
			}

	return imag;
}

/*************************************************************/
/*			          Function Eliminar Ruido	             */
/*************************************************************/
IplImage* eliminarRuido(IplImage *imag){
	
	switch(tipo_ruido){
       case 1:
		   imag = filtro_minimo(imag);
		   break;				   
       case 2:
		   imag = filtro_maximo(imag);
		   break;
	   case 3:
		   imag = filtro_medio(imag);
	   break;
	}
	return imag;


}

/*************************************************************/
/*	            Filtros para eliminar Ruidos	             */
/*************************************************************/

IplImage *filtro_maximo(IplImage *imag){

	CvScalar s1;
	IplImage *clon=cvCloneImage(imag);
	int i, j;
	int posx,posy;
	int max=0;
	for(i=1; i<imag->height-1; i++){
			for(j=1; j<imag->width-1; j++)
			{
				for(posx=(i-1); posx<(i+1); posx++){
					for(posy=(j-1); posy<(j+1); posy++)
					{		
						s1 = cvGet2D(imag,posx,posy);
						if(max<s1.val[0]){
							max=s1.val[0];	
						}
					}	
				}
				s1.val[0]=max;
					cvSet2D(clon,i,j,s1);
					max=0;
			
			}
	
	}
	return clon;
}

IplImage *filtro_minimo(IplImage *imag){

	CvScalar s1;
	IplImage *clon=cvCloneImage(imag);
	int i, j;
	int posx,posy;
	int max=80;
	for(i=1; i<imag->height-1; i++){
			for(j=1; j<imag->width-1; j++)
			{
				for(posx=(i-1); posx<(i+1); posx++){
					for(posy=(j-1); posy<(j+1); posy++)
					{		
						s1 = cvGet2D(imag,posx,posy);
						if(max>s1.val[0]){
							max=s1.val[0];	
						}
					}	
				}
				s1.val[0]=max;
					cvSet2D(clon,i,j,s1);
					max=0;
			
			}
	
	}
	return clon;
}

IplImage *filtro_medio(IplImage *imag){
	
	cvSmooth(imag,imag,CV_MEDIAN,3);
	return imag;
}

/*************************************************************/
/*	                    Menu Face Efect	                     */
/*************************************************************/

void menuFace_Efect(){
	int op;

	do{
		system("cls");
		
		printf("\n\n\n\t=================================================================");
		printf("\n\t==                                                             ==");
		printf("\n\t==                     F A C E - E F E C T                     ==");
		printf("\n\t==                     -------------------                     ==");
		printf("\n\t=================================================================\n\n\n");
		
		printf("\t\t M E N U - P R I N C I P A L ");
		printf("\n\t\t------------------------------");
		printf("\n\t\t\t 1. Iniciar\n");
		printf("\t\t\t 2. Salir");
		printf("\n\t\t\t Escoja la opcion Correcta: ");
		scanf("%d",&op);

	}while(op<1 || op>2);
	manejarOpciones(op);
}

/*************************************************************/
/*	                  Manejar Opciones       	             */
/*************************************************************/
void manejarOpciones(int op){

	switch(op){
			case 1:
				presentacioFaceEfect();
				break;
			case 2:
				exit(0);
				break;			
		}
}



//EDICION CONTRASTE Y BRILLO
void editorHist(IplImage *img){

	img_dst = cvCloneImage(img);

    if( !img )
    {
        printf("Image was not loaded.\n");
		return;
    }

		
    cvNamedWindow("image", 0);
	cvShowImage("image", img);

	crearHistogramas(img);

	cvCreateTrackbar("brightness", "image", &brightness, 200, actualizarBrillo);
	cvCreateTrackbar("contrast", "image", &contrast, 20, actualizarBrillo);
	//actualizarBrillo(brightness);
	
    cvWaitKey(0);



	cvReleaseImage(&img_dst);
	cvReleaseImage(&r);
	cvReleaseImage(&g);
	cvReleaseImage(&b);
	cvReleaseImage(&r2);
	cvReleaseImage(&g2);
	cvReleaseImage(&b2);

}


IplImage* Stretch(IplImage * img_in, double StretchFactor)
{
	IplImage * imag = cvCloneImage(img_in);
	CvScalar s1;
	int i, j;
	double Imax=0, Imin=255;
	
	
	cvMinMaxLoc( imag, &Imin, &Imax, NULL, NULL, NULL );
	//printf("min= %f, max= %f)\n",Imin,Imax);

	Imin = Imin+(Imax-Imin)*StretchFactor;
	Imax = Imax-(Imax-Imin)*StretchFactor;

	for(i=0; i<imag->height; i++){
			for(j=0; j<imag->width; j++)
			{
					s1 = cvGet2D(imag,i,j);
					double I = s1.val[0];
				
					I = ((I-Imin)/(Imax-Imin))*255;
					s1.val[0] = I;
					cvSet2D(imag, i,j, s1);

			}
	}	
	return imag;

}

IplImage* Desplazamiento(IplImage * img_in, int offset)
{
	CvScalar s1;
	IplImage * imag = cvCloneImage(img_in);
	for(int i=0; i<imag->height; i++){
			for(int j=0; j<imag->width; j++)
			{
					s1 = cvGet2D(imag,i,j);
					double I = (s1.val[0])+offset;
					if(I>255) I=255;
					if(I<0) I=0;
					s1.val[0] = I;
					cvSet2D(imag, i,j, s1);

			}
	}	
	return imag;
}

IplImage* Shrink(IplImage * img_in, double ShrinkFactor)
{	
	IplImage * imag = cvCloneImage(img_in);
	double shrinkMin,shrinkMax;
	CvScalar s1;
	int i, j;
	double Imax, Imin;

	cvMinMaxLoc( imag, &Imin, &Imax, NULL, NULL, NULL );
	//printf("min= %f, max= %f)\n",Imin,Imax);

	shrinkMin = Imin + (Imax-Imin)*ShrinkFactor;
	shrinkMax = Imax - (Imax-Imin)*ShrinkFactor;

	for(i=0; i<imag->height; i++){
			for(j=0; j<imag->width; j++)
			{
					s1 = cvGet2D(imag,i,j);
					double I = s1.val[0];
				
					I = (((shrinkMax-shrinkMin)/(Imax-Imin))*(I-Imin))+shrinkMin;
					s1.val[0] = I;
					cvSet2D(imag, i,j, s1);
			}
	}	
	return imag;

}
void actualizarBrillo(int val){
	int brillo = brightness-100;
	int contraste = contrast -10;

	r2 = Desplazamiento(r,brillo);
	g2 = Desplazamiento(g,brillo);
	b2 = Desplazamiento(b,brillo);

	if(contraste<0){
		r2 = Shrink(r2,(contraste*-0.05));
		g2 = Shrink(g2,(contraste*-0.05));
		b2 = Shrink(b2,(contraste*-0.05));
	}
	if(contraste>0){
		r2 = Stretch(r2,(contraste*0.05));
		g2 = Stretch(g2,(contraste*0.05));
		b2 = Stretch(b2,(contraste*0.05));
	}

	graficarHistogramas();
}

void crearHistogramas(IplImage * img){
	int celdas= 256;							
		
		//Calcula el Histograma de una imagen 8U
		histR=cvCreateHist(1,&celdas,CV_HIST_ARRAY,NULL,1);
		histG=cvCreateHist(1,&celdas,CV_HIST_ARRAY,NULL,1);
		histB=cvCreateHist(1,&celdas,CV_HIST_ARRAY,NULL,1);

		r =r2= cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );
		g =g2= cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );
		b =b2= cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );

		cvSplit(img,b,g,r,0);
		cvSplit(img,b2,g2,r2,0);

		graficarHistogramas();
	
}

void graficarHistogramas(){

		float max_value = 0;
		float bin_val;//para valores obtenidos del histograma calculado
		int intensity;//valores obtenidos de histograma calculado, redondeados y normalizados

		cvCalcHist( &r2, histR, 0, NULL ); 
		cvCalcHist( &g2, histG, 0, NULL );
		cvCalcHist( &b2, histB, 0, NULL );


		//Preparando Img_resp histograma
		imgHistR =cvCreateImage(cvSize(256,256),IPL_DEPTH_8U, 1);
		imgHistG =cvCreateImage(cvSize(256,256),IPL_DEPTH_8U, 1);
		imgHistB =cvCreateImage(cvSize(256,256),IPL_DEPTH_8U, 1);

		imgHistR->origin=IPL_ORIGIN_BL;
		imgHistG->origin=IPL_ORIGIN_BL;
		imgHistB->origin=IPL_ORIGIN_BL;
		
				
		//seteando la imagen del histograma a cero para su color de fondo
		cvSetZero(imgHistR);
		cvSetZero(imgHistG);
		cvSetZero(imgHistB);
		
	
		//obtenemos el maximo valor del histograma
		cvGetMinMaxHistValue(histR, 0,&max_value, 0, 0 );
		//pintando de blanco las frecuencias obtenidas en el histograma sobre la imgHistSalida
		for(int i=0;i<256;i++){	
			bin_val= cvQueryHistValue_1D(histR,i);
			intensity= cvRound(bin_val*255/max_value);
			for(int frecuencia=0;frecuencia<intensity;frecuencia++)
				((uchar*)imgHistR->imageData)[frecuencia*imgHistR->widthStep+i]=255;
		}

		max_value = 0;

		//obtenemos el maximo valor del histograma
		cvGetMinMaxHistValue(histG, 0,&max_value, 0, 0 );
		//pintando de blanco las frecuencias obtenidas en el histograma sobre la imgHistSalida
		for(int i=0;i<256;i++){	
			bin_val= cvQueryHistValue_1D(histG,i);
			intensity= cvRound(bin_val*255/max_value);
			for(int frecuencia=0;frecuencia<intensity;frecuencia++)
				((uchar*)imgHistG->imageData)[frecuencia*imgHistG->widthStep+i]=255;
		}


		max_value = 0;
		//obtenemos el maximo valor del histograma
		cvGetMinMaxHistValue(histB, 0,&max_value, 0, 0 );
		//pintando de blanco las frecuencias obtenidas en el histograma sobre la imgHistSalida
		for(int i=0;i<256;i++){	
			bin_val= cvQueryHistValue_1D(histB,i);
			intensity= cvRound(bin_val*255/max_value);
			for(int frecuencia=0;frecuencia<intensity;frecuencia++)
				((uchar*)imgHistB->imageData)[frecuencia*imgHistB->widthStep+i]=255;
		}

		cvMerge(b2,g2,r2,0,img_dst);

	cvNamedWindow("image2", 0);
	cvShowImage("image2", img_dst);


	cvNamedWindow("histogramR", 0);
	cvShowImage("histogramR",imgHistR);
	cvNamedWindow("histogramG", 0);
	cvShowImage("histogramG",imgHistG);
	cvNamedWindow("histogramB", 0);
	cvShowImage("histogramB",imgHistB);

}


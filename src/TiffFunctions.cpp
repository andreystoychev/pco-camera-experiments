#include "TiffFunctions.h"


char pcotiff_text[70] = "";


void store_tiff(const char *filename, int width, int height, int colormode, void *bufadr, char *apptext) {
	unsigned short *cptr;
	unsigned short *c1;
	unsigned int *b1;
	int hfstore;
	int e, z, x;
	int headerl;

	cptr = (unsigned short *)malloc(65536);

	if (cptr == NULL)
		return;

	hfstore = _open(filename, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY, 0666);//S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
	if (hfstore == -1)
	{
		free(cptr);
		return;
	}

	int slen, txtlen;
	char *ch;
	slen = (int)strlen(apptext);
	txtlen = slen + 1;
	txtlen = (txtlen / 16) * 16 + 16;

	c1 = cptr;
	*c1++ = 0x4949;           /* Begin TIFF-Header II */
	*c1++ = 0x002A;

	*c1++ = 0x0010;           /* Pointer to IFD  */
	*c1++ = 0;


	*c1++ = 0;
	*c1++ = 0;
	*c1 = 0;

	/* create  IFD */
	c1 = cptr + 8;

	*c1++ = 0x000F;             /* Entry Count */

	*c1++ = 0x00FE;             /* NewSubfileType */
	*c1++ = 0x0004;
	b1 = (unsigned int *)c1;
	*b1++ = 0x00000001;
	*b1++ = 0x00000000;
	c1 = (unsigned short *)b1;

	*c1++ = 0x0100;             /* ImageWidth */
	*c1++ = 0x0004;
	b1 = (unsigned int *)c1;
	*b1++ = 0x00000001;
	*b1++ = width;
	c1 = (unsigned short *)b1;

	*c1++ = 0x0101;             /* ImageHeight */
	*c1++ = 0x0004;
	b1 = (unsigned int *)c1;
	*b1++ = 0x00000001;
	*b1++ = height;
	c1 = (unsigned short *)b1;

	*c1++ = 0x0102;             /* BitsPerPixel */
	*c1++ = 0x0003;             /* SHORT */
	*c1++ = 0x0001;
	*c1++ = 0x0000;
	*c1++ = 0x0010;             /* 16 */
	*c1++ = 0x0000;

	*c1++ = 0x0103;             /* Compression */
	*c1++ = 0x0003;             /* SHORT */
	*c1++ = 0x0001;
	*c1++ = 0x0000;
	*c1++ = 0x0001;             /* 1 */
	*c1++ = 0x0000;

	*c1++ = 0x0106;             /* PhotometricInterpretation */
	*c1++ = 0x0003;             /* SHORT */
	*c1++ = 0x0001;
	*c1++ = 0x0000;
	*c1++ = 0x0001;             /* 1 */
	*c1++ = 0x0000;


	*c1++ = 0x0111;             /* StripOffset */
	*c1++ = 0x0004;
	b1 = (unsigned int *)c1;
	//@a  *b1++ = height/4;           /* 4 Zeilen pro */
	*b1++ = height;             /* 1 Zeilen pro */
	*b1++ = 0x00E0;              /* pointer */
	c1 = (unsigned short *)b1;

	*c1++ = 0x0115;             /* SamplePerPixel */
	*c1++ = 0x0003;             /* SHORT */
	*c1++ = 0x0001;
	*c1++ = 0x0000;
	*c1++ = 0x0001;             /* 1 */
	*c1++ = 0x0000;

	*c1++ = 0x0116;             /* RowsPerStrip */
	*c1++ = 0x0004;
	b1 = (unsigned int *)c1;
	*b1++ = 0x00000001;
	*b1++ = 0x00000001;
	c1 = (unsigned short *)b1;

	*c1++ = 0x0117;              /* StripByteCounts */
	*c1++ = 0x0004;
	b1 = (unsigned int *)c1;
	//@a  *b1++ = height/4;
	*b1++ = height;
	//@a  *b1++ = 0x0E0+height;        /* pointer */
	*b1++ = 0x0E0 + height * 4;        /* pointer */
	c1 = (unsigned short *)b1;

	*c1++ = 0x011A;              /* X-Resolution */
	*c1++ = 0x0005;
	b1 = (unsigned int *)c1;
	*b1++ = 0x00000001;
	//@a  *b1++ = 0x0E0+height*2;      /* pointer */
	*b1++ = 0x0E0 + height * 8;      /* pointer */
	c1 = (unsigned short *)b1;

	*c1++ = 0x011B;              /* Y-Resolution */
	*c1++ = 0x0005;
	b1 = (unsigned int *)c1;
	*b1++ = 0x00000001;
	//@a  *b1++ = 0x0E0+height*2+8;    /* pointer */
	*b1++ = 0x0E0 + height * 8 + 8;    /* pointer */
	c1 = (unsigned short *)b1;

	*c1++ = 0x011C;              /* PlanarConfiguration */
	*c1++ = 0x0003;              /* SHORT */
	*c1++ = 0x0001;
	*c1++ = 0x0000;
	*c1++ = 0x0001;              /* 1 */
	*c1++ = 0x0000;

	*c1++ = 0x0128;              /* ResolutionUnit */
	*c1++ = 0x0003;              /* SHORT */
	*c1++ = 0x0001;
	*c1++ = 0x0000;
	*c1++ = 0x0001;              /* 1 */
	*c1++ = 0x0000;

	*c1++ = 0x0131;              /* Software */
	*c1++ = 0x0002;
	b1 = (unsigned int *)c1;
	//  *b1++ = 0x00000046;
	*b1++ = slen + 1;
	*b1++ = 0x0E0 + height * 8 + 16;   /* pointer */


									   //fill with 0 until strips at 0xE0
	c1 = (unsigned short *)b1;
	for (; c1 < cptr + 0xE0 / 2;)
		*c1++ = 0;                   /* ende */

									 /* beginn der stripadressen 224byte vom Fileanfang */
									 //@a  z=0x0E0+height*2+16+70;     /*  70 is textlength */
	z = 0x0E0 + height * 8 + 16 + txtlen; // 70;     /*  70 is textlength */

	c1 = cptr + 0x70;  //0x70=0xE0/2
	b1 = (unsigned int *)c1;
	//@a  for(x=0;x<height/4;x++)
	//@a	*b1++=z+x*width*8;
	for (x = 0; x < height; x++)
		*b1++ = z + x*width * 2;

	//@a  for(x=0;x<height/4;x++)
	//@a   *b1++=width*8;
	for (x = 0; x < height; x++)
		*b1++ = width * 2;

	*b1++ = 0x00000004;
	*b1++ = width;
	*b1++ = 0x00000004;
	*b1++ = height;

	strcpy((char*)b1, apptext);
	ch = (char*)b1;
	ch += slen;
	for (; ch < (char*)cptr + z;)
		*ch++ = 0;

	headerl = (int)((ch - (char*)cptr));

	e = _write(hfstore, (void *)cptr, headerl);
	if (e == -1)
	{
		_close(hfstore);
		remove(filename);
		free(cptr);
		return;
	}

	z = width*height * 2;
	e = _write(hfstore, (void *)bufadr, z);
	if (e == -1)
	{
		_close(hfstore);
		remove(filename);
		free(cptr);
		return;
	}

	_close(hfstore);
	free(cptr);
	return;
}

void store_tiff(const char *filename, int width, int height, int colormode, void *bufadr) {
	if (strlen(pcotiff_text) == 0) {
		int x;
		sprintf(pcotiff_text, "PCO 16bit File-Program Copyright (C)2016 PCO ");
		if (colormode == 0)
			strcat(pcotiff_text, "Version: B/W    ");
		else
			strcat(pcotiff_text, "Version: COLOR  ");
		for (x = (int)strlen(pcotiff_text); x < 70 - 1; x++)
			pcotiff_text[x] = 0x20;
		pcotiff_text[x] = 0;
	}
	store_tiff(filename, width, height, colormode, bufadr, pcotiff_text);
}
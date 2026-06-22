#ifndef overlay_h
#define overlay_h

typedef struct overlay overlay;

overlay *overlaycreate(int w, int h, int x, int y);
void overlaydestroy(overlay *ov);
void overlaybegin(overlay *ov);
void overlayend(overlay *ov);
void overlaydrawtext(overlay *ov, int x, int y, char *text, float r, float g, float b, float size);
void overlaydrawrect(overlay *ov, int x, int y, int w, int h, float r, float g, float b, int fill);
void overlaydrawline(overlay *ov, int x1, int y1, int x2, int y2, float r, float g, float b);
void overlaydrawcircle(overlay *ov, int cx, int cy, int radius, float r, float g, float b, int fill);
void overlaydrawimage(overlay *ov, int x, int y, char *path);
void overlaysettitle(overlay *ov, char *title);
void overlaysetposition(overlay *ov, int x, int y);
void overlaysetsize(overlay *ov, int w, int h);
void overlayfocus(overlay *ov);
void overlayhide(overlay *ov);
void overlayshow(overlay *ov);

#endif

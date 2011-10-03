#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<windows.h>

typedef struct node
{
int type;
char *string;
struct node *next;
struct node *child;
} NODE;

char const cls[]="myclass";
FILE *f;

typedef struct stack
{
char c;
struct stack *next;
} STR_STACK;

typedef struct parent
{
struct node *node;
struct parent *next;
} PATH_STACK;

typedef struct tags
{
char *tag;
struct tags *next;
} TAG_STACK;

struct linkRect{
	int xpos1;
	int xpos2;
	int ypos1;
	int ypos2;
}linkRectindex[100];

struct color{
	int red;
	int green;
	int blue;
}txtColor,bgColor;

int sRangeY;
int sRangeX;
int maxSize=18;
int csY=0,csX=0,curX=0,curY=0,startX=2,startY=0;
int pagesizeY=0,pagesizeX=0,initlinkX,initlinkY,linkIndexpos=0;

HWND h;
PATH_STACK *PATH;
NODE *document;
RECT wndMetrics={0,0,1000,600},wndRect={0,0,1000,600};
char linkIndexchar[100][50];
char linkaddress[50];
LOGFONT fontsettings={0};
PAINTSTRUCT ps;
RECT sr;
HFONT hfont;
HGDIOBJ holdfont;
HDC hdc;
HINSTANCE hInst;


SIZE pSize,cSize;

char *args;
char tag[200]={0};
void createNode(int,char*);
void display();
void clear_vars();
int DOM_load(char*);
void DOM_init();

void pathStack_push(NODE *node)		
{
	PATH_STACK *n_frame;
	n_frame=(PATH_STACK*)malloc(sizeof(PATH_STACK));
	n_frame->node=node;
	n_frame->next=PATH;
	PATH=n_frame;
}

void pathStack_pop()
{
	PATH_STACK *temp;
	temp=PATH->next;
	free(PATH);
	PATH=temp;

}

char *getAttribute(NODE *node,char *attribute)		//method for extracting an attribute value
{
	NODE *child;
	char c,*str,*attstring,*attvalue;
	int i=0,k=0;
	child=node->child;
	while(child!=NULL)
	{
		if(child->type==2)		//this searches for an attribute node 
		{
recheck:
			k=0;
			str=(char*)malloc(strlen(child->string));
			attstring=(char*)malloc(strlen(child->string));
			attvalue=(char*)malloc(strlen(child->string));
			strcpy(str,child->string);
			c=str[i];
			while(c!='=')
			{	
				attstring[k]=c;
				i++;
				k++;
				c=str[i];
			}
			attstring[k]=0;
			k=0;
			i++;
			i++;
			c=child->string[i];
			while(c!='"')		//extracting the value set to an attribute 
			{
				attvalue[k]=c;
				i++;
				k++;
				c=child->string[i];
			}
			attvalue[k]=0;
			i++;
			c=child->string[i];
			if(strcmp(attstring,attribute)==0)
			{
				return attvalue;		// returns the value set to the attribute 
			}
			else if(c==' ')
			{
				i++;
				goto recheck;
			}
			else
			{	
				return NULL;
			}

		}
		child=child->next;
	}
	return NULL;		//return NULL if any attribute node or the requested attribute not found		
}

NODE *getElementsByTagName(NODE *parent,char *tag,int index)
{
	NODE *child1,*child2,*child3;
	char *testtag;
	child1=parent->child;
	while(child1!=NULL)
	{
		if(strcmp(tag,child1->string)==0 && child1->type==0)
		{
			return child1;
		}
		child2=getElementsByTagName(child1,tag,index);
		if(child2!=NULL) return child2;
		child1=child1->next;
	}
	return NULL;		
}

int DOM_Load(char *path)		//loads a document file
{

int fsize;

FILE *file;
file=fopen(path,"r");
if(!file) return 0;
fseek(file,0,SEEK_END);
fsize=ftell(file);
args=(char*)malloc(sizeof(char)*(fsize));
strcpy(args,"");
fseek(file,0,SEEK_SET);
fread(args,2,fsize,file);

fclose(file);
return 1;

}

void DOM_init()
{
PATH_STACK *temp;
int i=0,k=0;
char c,str[]="document",*str2;

document=(NODE*)malloc(sizeof(NODE));		// document node initialization
PATH=(PATH_STACK*)malloc(sizeof(PATH_STACK));
temp=(PATH_STACK*)malloc(sizeof(PATH_STACK));
document->type=0;
document->string=(char*)malloc(8);
document->next=NULL;
document->child=NULL;
document->string=str;

PATH->next=NULL;		//pushing document node into the path stack
PATH->node=document;

while(i<(strlen(args)-1))		//character to character parsing starts here
{	
    c=args[i];
    clear_vars();
    k=0;
    if(c=='\n') c=args[++i];
			
    if(c=='<')		//determines the start of an element tag
	{
		
		i++;
		c=args[i];
		
		if(c=='/')		//determines the closing tag of an element
		{	

			i++;
			c=args[i];
			pathStack_pop(); 
			while(c!='>')		//reading the contents of the tag till the brackets close
			{
				tag[k]=args[i];
				i++;
				k++;
				c=args[i];
			}
                        if(strcmp(tag,"html")==0) {i=strlen(args);break;}
		}
		else
		{
			while(c!='>')
			{	
				tag[k]=args[i];
				i++;
				k++;
				c=args[i];
                if(c==' ') break;	//break for attribute string reading
            }
			createNode(0,tag);		//create a node of type element with string=(tag contents)
			if(c==' ')
			{
					
				k=0;
				i++;
				clear_vars();
				while(c!='>')
				{
					tag[k]=args[i];
					k++;
		    		i++;
				
					c=args[i];
				
				}

				createNode(2,tag);		//create an attribute node
			}

		}
	}
	else	       	//here contents of a text node are read till a tag starts
	{
		while(c!='<')
		{
			tag[k]=args[i];
			k++;
			i++;
			c=args[i];
		}
                i--;
		createNode(1,tag);
	}
	i++;
}

document=PATH->node;

}
void createNode(int type,char *string)
{
	
NODE *n_node,*current,*parent;
PATH_STACK *n_parent;
char *c;
int x;
c=(char*)malloc(strlen(string));
strcpy(&c,&string);
n_node=(NODE*)malloc(sizeof(NODE)+strlen(string));	//initializing the new node and a new PATH stack frame
n_node->string=(char*)malloc(strlen(string));
n_parent=(PATH_STACK*)malloc(sizeof(PATH_STACK));
parent=(NODE*)malloc(sizeof(NODE));
current=(NODE*)malloc(sizeof(NODE));
n_node->type=type;
strcpy(n_node->string,c);
n_node->child=NULL;
n_node->next=NULL;
parent=PATH->node; //settting the parent node to the top node in the PATH stack

if(parent->child==NULL)
{		
	parent->child=n_node;
}
else
{	
	current=parent->child;
	while(current->next!=NULL)
	{
		current=current->next;
	}
    current->next=n_node;
}

if(type==0)
{
	pathStack_push(n_node);		//push the new node into the PATH stack
	
}
}
void clear_vars()
{
int k;
for(k=0;k<=200;k++)
{
	tag[k]=0;
}
}

void display()
{
NODE *current,*ccurrent;
current=document;
while(current!=NULL)
{
	printf("%s\n",current->string);
	ccurrent=current->next;
	while(ccurrent!=NULL)
	{
		printf("%s\n",ccurrent->string);
		ccurrent=ccurrent->next;
	}
	current=current->child;
}
}
void traverse(NODE *parent)		//recursive method to traverse the document tree starting with node=(parent)
{
NODE *child;char *c;
child=parent->child;
while(child!=NULL)
{	
	MessageBox(0,child->string,"h",0);
	traverse(child);
	child=child->next;
}
}
void SetColorFromString(char *color)
{
	if(color[0]!='#')
	{
		if(strcmp(color,"BLACK")==0) 
		{
			txtColor.blue=txtColor.red=txtColor.green=0;
		}
		if(strcmp(color,"WHITE")==0) 
		{
			txtColor.blue=txtColor.red=txtColor.green=255;
		}
		if(strcmp(color,"RED")==0)
		{
			txtColor.red=255;
			txtColor.blue=txtColor.green=0;
		}
		if(strcmp(color,"GREEN")==0)
		{
			txtColor.green=255;
			txtColor.blue=txtColor.red=0;
		}

		if(strcmp(color,"BLUE")==0)
		{
			txtColor.blue=255;
			txtColor.green=txtColor.red=0;
		}
	}
}
	
int getIntFromString(char *string)
{
	char c,*str;
	int x=0,i=0,len;
	len=strlen(string)-2;
	str=(char*)malloc(strlen(string));
	strcpy(str,string);
	c=str[i];
	while(i<=len)
	{
		c=str[len-i];
		x=x+(((int)(c-48))*(10^i));
		
		i++;
	}
	return x;
}

int lookup_link_metrics(int x,int y)		//fetches index for an hyperlink rectangle
{
	int k=0;
	while(k<100)
	{
		if (x>linkRectindex[k].xpos1 && x<linkRectindex[k].xpos2 && y>linkRectindex[k].ypos1 && y<linkRectindex[k].ypos2) break;
		k++;
	}
	return k;
}

void set_scrollbar()		//initializes scroll bar settings (range,unit size )
{
	int wndsizeX,wndsizeY;
	wndsizeX=wndRect.right;
	wndsizeY=wndRect.bottom;
	if (pagesizeY>wndsizeY) {sRangeY=((pagesizeY-wndsizeY+50)/10);}
	else sRangeY=0;
	if(pagesizeX>wndsizeX){sRangeX=((pagesizeX-wndsizeX+50)/10);}
	else sRangeX=0;
	SetScrollRange(h,SB_VERT,0,sRangeY,FALSE);
	SetScrollRange(h,SB_HORZ,0,sRangeX,FALSE);		
}

void PushElementSettings(NODE *element)		// element property settings initialization
{
	char *elementstr;
	char *fontface,*fontsize,*fontcolor;
	elementstr=(char*)malloc(strlen(element->string));
	strcpy(elementstr,element->string);

	if(strcmp(elementstr,"font")==0)
	{
		if(getAttribute(element,"face")!=NULL)
		{
			strcpy(fontsettings.lfFaceName,getAttribute(element,"face"));
		}

		if(getAttribute(element,"size")!=NULL)
		{
			fontsettings.lfHeight=getIntFromString(getAttribute(element,"size"));
		}
		if(getAttribute(element,"color")!=NULL)
		{
			SetColorFromString(getAttribute(element,"color"));
		}

	}

	if(strcmp(elementstr,"b")==0)
	{
		fontsettings.lfWeight=700;
	}
	if(strcmp(elementstr,"i")==0)
	{
		fontsettings.lfItalic=1;
	}
	if(strcmp(elementstr,"p")==0)
	{
		curX=startX;
		curY+=maxSize;
	}
	if(strcmp(elementstr,"h1")==0)
	{

		fontsettings.lfWeight=700;
		fontsettings.lfHeight=44;
	}
	if(strcmp(elementstr,"h2")==0)
	{
		fontsettings.lfWeight=700;
		fontsettings.lfHeight=36;
		curY+=maxSize;
	}
	if(strcmp(elementstr,"a")==0)
	{
		initlinkX=curX;
		initlinkY=curY;
		strcpy(linkIndexchar[linkIndexpos],getAttribute(element,"href"));
		txtColor.blue=255;
		txtColor.red=0;
		txtColor.green=0;
	}
}
void PopElementSettings(char *element)		//element property settings un-initialization
{
	if(strcmp(element,"font")==0)
	{
		strcpy(fontsettings.lfFaceName,"Arial");
		fontsettings.lfHeight=20;
		SetColorFromString("BLACK");
	}

	if(strcmp(element,"b")==0)
	{
		fontsettings.lfWeight=0;
	}
	if(strcmp(element,"i")==0)
	{
		fontsettings.lfItalic=0;
	}
	if(strcmp(element,"p")==0)
	{
		
	}
	if(strcmp(element,"h1")==0)
	{
		fontsettings.lfWeight=0;
		fontsettings.lfHeight=18;
		curX=startX;
		curY+=5;
	
	}
	if(strcmp(element,"h2")==0)
	{
		fontsettings.lfWeight=0;
		fontsettings.lfHeight=18;
		curX=startX;
		curY+=5;
		
	}
	if(strcmp(element,"a")==0)
	{
		linkRectindex[linkIndexpos].xpos1=initlinkX;
		linkRectindex[linkIndexpos].xpos2=curX;
		linkRectindex[linkIndexpos].ypos1=initlinkY;
		linkRectindex[linkIndexpos].ypos2=initlinkY+maxSize;
		linkIndexpos++;	
		txtColor.blue=0;
		txtColor.red=0;
		txtColor.green=0;
	}
}

void traverseBody(NODE *parent)		//traverses body element for text nodes and displays them
{	
	NODE *child;
	HRGN rgn;
	child=parent->child;

	while(child!=NULL)
	{

		if(child->type==0)
		{
			PushElementSettings(child);
		}
		
		if(child->type==1)
		{
			hfont=CreateFontIndirect(&fontsettings);
			holdfont=SelectObject(hdc,hfont);
			GetTextExtentPoint32(hdc,child->string,strlen(child->string),&cSize);
			SetTextColor(hdc,RGB(txtColor.red,txtColor.green,txtColor.blue));
			if (cSize.cy<=maxSize)
				{
				
					TextOut(hdc,curX,curY+(0.8*(maxSize-cSize.cy)) ,child->string,strlen(child->string));
				}
				else
				{
					sr.top=curY;
					sr.left=0;
					sr.bottom=curY+maxSize;
					sr.right=curX;
					rgn=CreateRectRgn(sr.left,sr.top,sr.right,sr.bottom);
					ScrollWindow(h,0,0.80*(cSize.cy-maxSize),&sr,NULL);
					maxSize=cSize.cy;			
					TextOut(hdc,curX,curY,child->string,strlen(child->string));
				}
			if(curX>990) {maxSize=20;curX=startX; curY=curY+maxSize+10;}
			curX=curX+cSize.cx;
			SelectObject(hdc,holdfont);
			DeleteObject(hfont);
			if(pagesizeX<curX) pagesizeX=curX;
			pagesizeY=curY;
		}
		traverseBody(child);
		PopElementSettings(child->string);
		child=child->next;
	}
}

void RenderContent()		//body element traversal entry point
{
	NODE *temp;
	int i=0;
	temp=getElementsByTagName(document,"body",0);
	hdc=BeginPaint(h,&ps);
	curX=startX;
	curY=startY;
	while(i<=100)
	{
		linkRectindex[i].xpos1=0;
		linkRectindex[i].xpos2=0;
		linkRectindex[i].ypos1=0;
		linkRectindex[i].ypos2=0;
		i++;
	}
	linkIndexpos=0;
	traverseBody(temp);
}

void LoadAddress(char *address)
{
	char *title;
	DOM_Load(address);
	DOM_init();
	title=(char*)malloc(strlen(getElementsByTagName(document,"title",0)->child->string));
	strcpy(title,getElementsByTagName(document,"title",0)->child->string);
	strcat(title," - HTML Parser");
	RenderContent();
	curX=startX=pagesizeX=5;
	startY=curY=pagesizeY=0;
	SetWindowText(h,title);
	set_scrollbar();
}
	
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) //window procedure function
{
int lIndex;
char link_address[50];
    switch(msg)
    {
	case WM_LBUTTONDOWN:
		lIndex=lookup_link_metrics(LOWORD(lParam),HIWORD(lParam));
		if (lIndex<100)
		{	
			strcpy(link_address,linkIndexchar[lIndex]);
			LoadAddress(link_address);
			InvalidateRect(hwnd,&wndRect,TRUE);
		}
		break;
	case WM_MOUSEMOVE:
		lIndex=lookup_link_metrics(LOWORD(lParam),HIWORD(lParam));
		if(lIndex<100)
		{
			SetCursor(LoadCursor(NULL,MAKEINTRESOURCE(32649)));
		}
		else SetCursor(LoadCursor(NULL,IDC_ARROW));
		break;

	case WM_HSCROLL:
		csX=GetScrollPos(h,SB_HORZ);
		switch (LOWORD(wParam))
		{
		case SB_LINELEFT:
		if (csX>0)
		{	
			SetScrollPos(h,SB_HORZ,csX-1,FALSE);
			startX=startX+10;
			InvalidateRect(hwnd,&wndRect,TRUE);
			
			break;
		}
			break;
		case SB_LINERIGHT:
		if (csX<sRangeX)
		{
			
			SetScrollPos(h,SB_HORZ,csX+1,FALSE);
			startX=startX-10;
			InvalidateRect(hwnd,&wndRect,TRUE);
			
			break;
		}
			break;
		default:
			break;
		}


	case WM_VSCROLL:

		csY=GetScrollPos(h,SB_VERT);
		
		switch (LOWORD(wParam))
		{
		case SB_LINEUP:
			if (csY>0)
			{	
				SetScrollPos(h,SB_VERT,csY-1,FALSE);
				startX=0;
				startY=startY+10;
				InvalidateRect(hwnd,&wndRect,TRUE);
				
				break;
			}
			break;
		case SB_LINEDOWN:
			if (csY<(sRangeY))
			{
				
				SetScrollPos(h,SB_VERT,csY+1,FALSE);
				startX=0;
				startY=startY-10;
				InvalidateRect(hwnd,&wndRect,TRUE);
				
				break;
			}
			break;
		default:

			break;
		}
		break;
		case WM_PAINT:
paint:
			GetWindowRect(hwnd,&wndMetrics);
			wndRect.right=wndMetrics.right-wndMetrics.left;
			wndRect.bottom= wndMetrics.bottom-wndMetrics.top ; 
			InvalidateRect(h,&wndRect,TRUE);
			RenderContent();
			UpdateWindow(h);
			set_scrollbar();
		

			break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}


int _stdcall WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
WNDCLASSEX wc;
MSG m;
NODE *n;
HCURSOR cur;
char *title;
wc.cbSize=sizeof(WNDCLASSEX);
wc.style=0;
wc.lpfnWndProc=WndProc;
wc.cbClsExtra=0;
wc.cbWndExtra=0;
wc.hInstance=0;
wc.hIcon=NULL;
wc.hCursor=NULL;
wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
wc.lpszMenuName=NULL;
wc.lpszClassName=cls;
wc.hIconSm=NULL;
RegisterClassEx(&wc);

h=CreateWindowEx(WS_EX_CLIENTEDGE,cls,"HTML Parser",WS_HSCROLL|WS_VSCROLL|WS_OVERLAPPEDWINDOW|WS_MAXIMIZE,0,0,1000,600,NULL,NULL,NULL,NULL);
ShowWindow(h,nShowCmd);
LoadAddress("D:\\a2.htm");

while(GetMessage(&m, NULL, 0, 0) > 0)
    {
        TranslateMessage(&m);
        DispatchMessage(&m);
    }
	return(0);
}

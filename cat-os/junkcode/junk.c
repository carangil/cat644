/*
 * junk.c
 *
 * Created: 1/19/2015 7:48:31 PM
 *  Author: mark
 */ 
#if 0

/* Where to put unused stuff */

void simple_ram_test() {
	
	//ram test
	SELECT_RAM_BANK(0);
	
	SELECT_RAM_PAGE(1);
	START_FAST_WRITE;
	FAST_WRITE(10, 100);
	FAST_WRITE(20, 200);
	FAST_WRITE(30, 250);
	END_FAST_WRITE;
	
	SELECT_RAM_PAGE(2);
	START_FAST_WRITE;
	FAST_WRITE(10, 101);
	FAST_WRITE(20, 201);
	FAST_WRITE(30, 251);
	END_FAST_WRITE;
	
	SELECT_RAM_BANK(1);
	SELECT_RAM_PAGE(1);
	START_FAST_WRITE;
	FAST_WRITE(10, 110);
	FAST_WRITE(20, 210);
	FAST_WRITE(30, 250);
	END_FAST_WRITE;
	
	SELECT_RAM_PAGE(2);
	START_FAST_WRITE;
	FAST_WRITE(10, 111);
	FAST_WRITE(20, 211);
	FAST_WRITE(30, 251);
	END_FAST_WRITE;
	
	{
		{
			char bank;
			char page;
			char a,b,c;
			
			for (bank=0;bank<2;bank++) {
				for(page=1;page<3;page++) {
					SELECT_RAM_BANK(bank);
					SELECT_RAM_PAGE(page);
					
					FAST_READ(a,10);
					FAST_READ(b,20);
					FAST_READ(c,30);
					
					sprintf(str, "%d %d %d %d %d ", bank, page, a,b,c );
					prints(&dev_comm, str);
					
				}
			}
			
			
		}
	}
	
	
}




unsigned char catsprite[] =
{
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,
	0,1,1,0,0,0,0,0,0,0,0,0,1,0,1,0,
	0,1,0,0,1,0,0,0,0,0,0,1,0,0,1,0,
	0,1,0,0,0,1,1,1,1,1,1,0,0,0,1,0,
	0,1,0,1,1,1,0,0,0,0,1,1,1,0,1,0,
	0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,
	0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,
	0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,
	0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,
	0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
	0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,
	0,0,1,0,0,0,0,1,1,0,0,0,0,1,0,0,
	0,0,0,1,1,1,0,0,0,0,1,1,1,TRANSPARENT,TRANSPARENT,TRANSPARENT,
	0,0,0,0,0,0,1,1,1,1,0,0,0,TRANSPARENT,TRANSPARENT,TRANSPARENT,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};


	#if 0
	{
		int j;
		int r;
		
		for(j=0;j<10;j++)	{
			
			
			
			r= sdcard_read_block(0,j,diskbuffer, SDCARD_BLOCKSIZE);
			sprintf(str, "read block %d  result: %d", (int)j, r);
			prints(&dev_comm, str);
			{
				
				int i;
				for (i=0;i<512;i++) {
					sprintf(str, "%x(%c) ", diskbuffer[i] , diskbuffer[i]>32? diskbuffer[i]: ' ' );
					prints(&dev_comm, str);
				}
			}
			
			
			
		}
		
		
		sprintf(diskbuffer, "This is a block!");
		diskbuffer[509]='A';
		diskbuffer[510]='B';
		diskbuffer[511]='C';
		
		j=4;
		r= sdcard_write_block(0,j, diskbuffer, SDCARD_BLOCKSIZE);
		sprintf(str, "write block %d  result: %d", (int)j, r);
		
		for(j=0;j<10;j++)	{
			
			
			
			r= sdcard_read_block(0,j,diskbuffer, SDCARD_BLOCKSIZE);
			sprintf(str, "read block %d  result: %d", (int)j, r);
			prints(&dev_comm, str);
			{
				
				int i;
				for (i=0;i<512;i++) {
					sprintf(str, "%x(%c) ", diskbuffer[i] , diskbuffer[i]>32? diskbuffer[i]: ' ' );
					prints(&dev_comm, str);
				}
			}
			
			
			
		}
		
	}
	#endif


	while(1){
				drawsprite( x, y, catsprite, 16, 16);			
				
				drawsprite( x, y+20, catsprite, 16, 16);
				drawsprite( x, y+40, catsprite, 16, 16);
				drawsprite( x, y+60, catsprite, 16, 16);
				drawsprite( x, y+80, catsprite, 16, 16);
				drawsprite( x, y+100, catsprite, 16, 16);
				
				drawsprite( x, y+120, catsprite, 16, 16);
				drawsprite( x, y+140, catsprite, 16, 16);
				drawsprite( x, y+160, catsprite, 16, 16);
				
				
				x+=1;
				if (x==0)
					y+=1;
				
					
				
			}
		
		}
		
		#if 1
		{

	prints(&dev_keyincommout, "pressenter");
	dev_keyincommout.getc(0);
			
			SELECT_RAM_BANK(0);
			START_FAST_WRITE;
			for(i=0;i<240;i++) {
				
				SELECT_RAM_PAGE(i);
				
				for(j=0;j<240;j++) {
					FAST_WRITE(j,7);
				
				}
			}
			
			vga_fast();
			
			prints(&dev_keyincommout, "pressenter2");
			dev_keyincommout.getc(0);
			vga_mode(VGA_512);
			
			SELECT_RAM_BANK(1);
			START_FAST_WRITE;
			for(i=0;i<240;i++) {
				SELECT_RAM_PAGE(i);
				for(j=0;j<240;j++) {
					FAST_WRITE(j,63);
					FAST_WRITE(j,63);
					FAST_WRITE(j,63);
					FAST_WRITE(j,63);
					FAST_WRITE(j,63);
					FAST_WRITE(j,63);
					FAST_WRITE(j,63);
				}
			}
			
			END_FAST_WRITE;
		}
		
			prints(&dev_keyincommout, "pressenter3");
			dev_keyincommout.getc(0);
				
		
			
			
			prints(&dev_keyincommout, "pressenter3");
			dev_keyincommout.getc(0);
			
			vga_mode(VGA_256);
		
		#endif
			
		VGA_BEGIN_DRAW;	
		for (i=20;i<200;i++)
		{
			
			j=i>>1;
			VGA_DOT(i,j,i);	
		}
		VGA_END_DRAW;
			
		vga_slow();
			
		//keyboard/comm test
		while (1){
			/*
			prints(&dev_comm, "This is a test comm>");
			readline(&dev_comm, str, sizeof(str), 1);
			prints(&dev_comm, "You said ");
			prints(&dev_comm, str);
			while (! dev_comm.kbhit())
				prints(&dev_comm, ".");
			*/
				
			prints(&dev_keyincommout, "This is a test keyb>");
			readline(&dev_keyincommout, str, sizeof(str), 1);
			prints(&dev_keyincommout, "You said ");
			prints(&dev_keyincommout, str);
			while (! dev_keyincommout.kbhit(0))
				prints(&dev_keyincommout, ".");
				
				
				
			VGA_DISPLAY(dpage);
			
			
			//SELECT_RAM_BANK(dpage);
			
			sprintf(str, "dpage:%d", dpage);
			prints(&dev_comm, str);
			
			dpage++;
			if (dpage==2)
				dpage=0;
				
		}
#endif
    kernel void blur (global float* input,
					 global float* output,
			         int w,
					 int h, 
					 int radius
					)
    {
        int col_index = get_global_id (0);
        int row_index = get_global_id (1);
		int arrayLength=w*h;
		int index_current_channel=col_index + (row_index*w);
		int index_alto_sx=index_current_channel -(w*radius)-(3*radius);
			
		int i,j,cnt=0;
		float sum=0.0;
		int diam=(radius*2)+1;
		for(i=index_alto_sx; i<=index_alto_sx+(diam*3);i=i+3){
			for(j=0; j<diam; j++){
				int current_index=i+(j*w);
				if(current_index>0 && current_index<arrayLength){
					cnt++;
					sum=sum+input[current_index];	
				}	
			}
		}		
		if(cnt>0){
			float avg=(float)(sum/(float)cnt);
			output[index_current_channel]=avg;		
		}				
	}
	
	kernel void pump_up_red (global float* input,
					 global float* output,
			         int w,
					 int h )
    {
        int col_index = get_global_id (0);
        int row_index = get_global_id (1);

		int col_index_local = get_local_id (0);
        int row_index_local = get_local_id (1);

		if(col_index_local==0){
			
			output[col_index + row_index*w] =1;
		}
		else{
			output[col_index + row_index*w] = input[col_index + row_index*w];
		}
	}

	kernel void pump_up_green (global float* input,
					 global float* output,
			         int w,
					 int h )
    {
        int col_index = get_global_id (0);
        int row_index = get_global_id (1);

		int col_index_local = get_local_id (0);
        int row_index_local = get_local_id (1);

		//printf("x: %d \n",col_index);
		//printf("y: %d \n",row_index);

		//printf("x_local: %d \n",col_index_local );
		//printf("y_local: %d \n",row_index_local );
		
		if(col_index_local==1){
			
			output[col_index + row_index*w] =1;
		}
		else{
			output[col_index + row_index*w] = input[col_index + row_index*w];
		}
	}

	kernel void pump_up_blue (global float* input,
					 global float* output,
			         int w,
					 int h )
    {
        int col_index = get_global_id (0);
        int row_index = get_global_id (1);

		int col_index_local = get_local_id (0);
        int row_index_local = get_local_id (1);

		//printf("x: %d \n",col_index);
		//printf("y: %d \n",row_index);

		//printf("x_local: %d \n",col_index_local );
		//printf("y_local: %d \n",row_index_local );
		
		if(col_index_local==2){
			
			output[col_index + row_index*w] =1;
		}
		else{
			output[col_index + row_index*w] = input[col_index + row_index*w];
		}
	}

	 kernel void decrease_definition (global float* input,
					 global float* output,
			         int w,
					 int h )
    {
	//local 9 3
        int col_index = get_global_id (0);
        int row_index = get_global_id (1);
		int col_index_local = get_local_id (0);
        int row_index_local = get_local_id (1);

		if(col_index_local>=3 && col_index_local<=5 && row_index_local==1){
			int index=col_index + row_index*w;
			int index_canale_pixel_sinistra=index-3;
			int index_canale_pixel_destra=index+3;

			int index_canale_pixel_sopra=index-w;
			int index_canale_pixel_sopra_sx=index-w-3;
			int index_canale_pixel_sopra_dx=index-w+3;

			int index_canale_pixel_sotto=index+w;
			int index_canale_pixel_sotto_sx=index+w-3;
			int index_canale_pixel_sotto_dx=index+w+3;

			float media = (input[index]+ input[index_canale_pixel_sinistra]+ input[index_canale_pixel_destra]+ 
			input[index_canale_pixel_sopra]+ input[index_canale_pixel_sopra_sx]+ input[index_canale_pixel_sopra_dx]+ 
			input[index_canale_pixel_sotto]+input[index_canale_pixel_sotto_sx]+ input[index_canale_pixel_sotto_dx])/9;


			output[index]=media;
			output[index_canale_pixel_sinistra]=media;
			output[index_canale_pixel_destra]=media;
			output[index_canale_pixel_sopra]=media;
			output[index_canale_pixel_sopra_sx]=media;
			output[index_canale_pixel_sopra_dx]=media;
			output[index_canale_pixel_sotto]=media;
			output[index_canale_pixel_sotto_sx]=media;
			output[index_canale_pixel_sotto_dx]=media;
		}
	}
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdio.h>



#include <CL/cl.h>






void CheckError(cl_int error)
{
	if (error != CL_SUCCESS) {
		std::cerr << "OpenCL call failed with error " << error << std::endl;
		std::exit(1);
	}
}

std::string LoadKernel(const char* name)
{
	std::ifstream in(name);
	std::string result(
		(std::istreambuf_iterator<char>(in)),
		std::istreambuf_iterator<char>());
	return result;
}

cl_program CreateProgram(const std::string& source,cl_context context)
{
	size_t lengths[1] = { source.size() };
	const char* sources[1] = { source.data() };

	cl_int error = 0;
	cl_program program = clCreateProgramWithSource(context, 1, sources, lengths, &error);
	CheckError(error);

	return program;
}

bool read_bmp(char* filename, std::vector< float >& output, int& width, int& height)
{
	int i;
	FILE* f = fopen(filename, "rb");

	if (!f) return false;

	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header


	//leggi dim immagine
	width = *(int*)&info[18];
    height = *(int*)&info[22];
	
	//calcolo dim totale
	size_t size = 3 * width * height;

	//leggi buffer immagine
	std::vector< unsigned char > raw_data(size);
	fread(raw_data.data(), sizeof(unsigned char), size, f); // read the rest of the data at once
	fclose(f);

	//alloca matrice immagine
	output.resize(size);

	//swap BGR -> RGB e normalizza
	for (i = 0; i < size; i += 3)
	{
		output[i+0] = float(raw_data[i+2]) / 255.0f;
		output[i+1] = float(raw_data[i+1]) / 255.0f;
		output[i+2] = float(raw_data[i+0]) / 255.0f;
	}
	return true;
}

bool write_bmp(char* filename,const std::vector< float >& input, int width, int height)
{
	int i;
	FILE* f = fopen(filename, "wb");
	if (!f) return false;
	int filesize = 54 + 3 * width * height;

	unsigned char bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
	unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };

	int* ref_size   = (int*)&bmpfileheader[2];
	int* ref_width  = (int*)&bmpinfoheader[4];
	int* ref_height = (int*)&bmpinfoheader[8];

	(*ref_size)   = filesize;
	(*ref_width)  = width;
	(*ref_height) = height;
	
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);
	
	size_t size = 3 * width * height;
	std::vector< unsigned char > raw_data(size);
	//swap RGB -> BGR e denormalizza
	for (i = 0; i < size; i += 3)
	{
		raw_data[i + 0] = unsigned char(input[i + 2] * 255.0f);
		raw_data[i + 1] = unsigned char(input[i + 1] * 255.0f);
		raw_data[i + 2] = unsigned char(input[i + 0] * 255.0f);
	}
	fwrite(raw_data.data(), sizeof(unsigned char), size, f); // read the rest of the data at once
	fclose(f);
	return true;
}


int main(int argc, char **argv)
{
	int w, h;
	std::vector< float > image_in,image_out;
	
	if (!read_bmp("bosco.bmp", image_in, w, h))
	{
		std::cout << "Non riesco a caricare l'immagine" << std::endl;
		return 1;
	}
	
	size_t size = 3 * w * h;
	int widthMat = w * 3;
	int heightMat = h;
	int radius;
	image_out.resize(size);

	std::vector<char *> availableEffects(4);
	std::vector<char *> effects(50);
	effects[0] = "blur";
	effects[1] = "pump_up_red";
	effects[2] = "pump_up_blue";
	effects[3] = "pump_up_green";
	std::cout << "Quali effetti vuoi usare? [0] per blur \n [1] per saturare i rossi \n [2] per saturare i blu \n [3] per saturare i verdi \n [-1] per eseguire";
	int countEffects=0;
	int exit = 0;
	while (exit==0) {
		int userInput;
		std::cin >> userInput;		
		switch (userInput)
		{
			case 0: 
				effects[countEffects] = "blur";
				countEffects++;
				break;
			case 1:
				effects[countEffects] = "pump_up_red";
				countEffects++;
				break;
			case 2:
				effects[countEffects] = "pump_up_blue";
				countEffects++;
				break;
			case 3:
				effects[countEffects] = "pump_up_green";
				countEffects++;
				break;
			default:
				exit = 1;
				break;
		}
	}
	if (countEffects <= 0)
		return 1;

	std::cout << "Raggio?";
	std::cin >> radius;



	
	cl_uint platformIdCount = 0;
	clGetPlatformIDs(0, nullptr, &platformIdCount);
	std::vector<cl_platform_id> platformIds(platformIdCount);
	clGetPlatformIDs(platformIdCount, platformIds.data(), nullptr);

	
	cl_uint deviceIdCount = 0;
	clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_GPU, 0, nullptr,&deviceIdCount);
	std::vector<cl_device_id> deviceIds(deviceIdCount);
	clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_GPU, deviceIdCount,deviceIds.data(), nullptr);

	//initialize context
	const cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM,
		reinterpret_cast<cl_context_properties> (platformIds[0]),
		0, 0
	};
	cl_int error;
	cl_context context = clCreateContext(contextProperties, deviceIdCount,deviceIds.data(), nullptr,nullptr, &error);

	//create queue
	cl_command_queue queue = clCreateCommandQueueWithProperties(context, deviceIds[0], 0, &error);
	CheckError(error);

	cl_program program = CreateProgram(LoadKernel("image_effect_kernel.cl"), context);
	CheckError(clBuildProgram(program, deviceIdCount, deviceIds.data(), nullptr, nullptr, nullptr));

	

	cl_event eventsToWait[1];
	cl_event event1, event2;
	for (int i = 0; i < countEffects; ++i)
	{
		
		cl_kernel current_kernel=clCreateKernel(program, effects[i], &error);
		CheckError(error);

		//create Buffers
		cl_mem input = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * image_in.size(), image_in.data(), &error);
		CheckError(error);
		cl_mem output = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * image_out.size(), image_out.data(), &error);
		CheckError(error);

		//passing arguments to kernel
		clSetKernelArg(current_kernel, 0, sizeof(cl_mem), &input);
		clSetKernelArg(current_kernel, 1, sizeof(cl_mem), &output);
		clSetKernelArg(current_kernel, 2, sizeof(int), &widthMat);
		clSetKernelArg(current_kernel, 3, sizeof(int), &heightMat);
		clSetKernelArg(current_kernel, 4, sizeof(int), &radius);

		const size_t globalWorkSize[] = { widthMat, heightMat };
		const size_t localWorkSize[] = { 3, 1 };
		
		
		if (i == 0) {//non devo aspettare nessun evento
			CheckError(clEnqueueNDRangeKernel(queue, current_kernel, 2, nullptr, globalWorkSize, localWorkSize, 0, nullptr, &event1));
		}
		else {//aspetto l'evento della precedente iterazione
			CheckError(clEnqueueNDRangeKernel(queue, current_kernel, 2, nullptr, globalWorkSize, localWorkSize, 1, eventsToWait, &event1));
		}

		eventsToWait[0] = event1;
		
		CheckError(clEnqueueReadBuffer(queue, output, CL_TRUE, 0, sizeof(float) * image_out.size(), image_out.data(), 1, eventsToWait, &event2));
		eventsToWait[0] = event2;

		image_in = image_out;
	
		clReleaseMemObject(input);
		clReleaseMemObject(output);
		clReleaseKernel(current_kernel);
	}
	
	if (!write_bmp("bosco_o.bmp", image_out, w, h))
	{
		std::cout << "Non riesco a salvare l'immagine" << std::endl;
		return 1;
	}
	
	clReleaseCommandQueue(queue);

	
	clReleaseProgram(program);

	clReleaseContext(context);
	return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <ctypes.h>

static error_no file_ds_size(int file_ds, n64* size)
{
	struct stat file_stat;

	if(fstat(file_ds, &file_stat) != 0)
	{
		return errno;
	}

	*size = ()file_stat.st_size;
	return success;
}

static FString fstr_from_cstr(CCString cstr)
{
	FString fstr = {0};
	fstr.length = strlen(cstr);
	fstr.chars = cstr;
	return fstr;
}

typedef struct {
	n64 date;

	struct {
		FString status;
		f32 humidity;
		f32 temperature;
	} today;

	struct {
		FString errors;
		bool humidity_online;
		bool temperature_online;
	} sensors;

	struct {
		i8 day_off;
		FString status;
	} record[3];

} WeatherForecast;

error main(void)
{
#if 0
	n32 i;
#endif

	JSON_Field fields[] = {
		JSON_Field_add_number(WeatherForecast, date)
	};
	/* JSON_model_add_object(WeatherForecast, today); */
	/* JSON_model_add_object(WeatherForecast, sensors); */
	/* JSON_model_add_array(WeatherForecast, record); */


	n32 inputfile_size;
	CCString inputfile_path = "weatherforecast.json";
	int inputfile_ds = open(inputfile_path, O_RDONLY);
	if(inputfile_ds == -1)
	{
		fprintf(stderr, "ERROR:%s:%d: Could not open file %s: %s\n",
				__FILE__, __LINE__, inputfile_path, strerror(errno));
		return failure;
	}

	if(errno = file_ds_size(inputfile_ds, &inputfile_size))
	{
		fprintf(stderr, "ERROR:%s:%d: Could not get file size %s: %s\n",
				__FILE__, __LINE__, inputfile_path, strerror(errno));
		return failure;
	}


	(void)fields;
	printf("File %s of size %u\n", inputfile_path, inputfile_size);

#if 0
	for(i = 0; i < inputfile_size; ++i)
	{
		char curr;
		int raw_char = getchar(inputfile_ds);
		if(raw_char == EOF)
		{
			fprintf(stderr, "ERROR:%s:%d: Unexpected EOF at %d\n",
					__FILE__, __LINE__, i);
			return failure;
		}

		curr = (char)raw_char;
		JSON_parse_char(curr);
	}
#endif

	return success;
}

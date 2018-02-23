#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

int fileSize ( char File[] );
void fileNameExt ( char Name[], char Ext[], char File[] );

int main( int argc, char *argv[] )
{
	FILE *source, *destination;
	int size, parts;
	char name[100], ext[100], aux[100];

	if( argc < 3 )
	{
		printf( "There aren't enough args!
		" );
		system( "pause" );
		return 0;
	}

	fileNameExt( name, ext, argv[1] );
	parts = atoi( argv[2] );

	size = fileSize( argv[1] );
	if( (source = fopen(argv[1], "rb")) == NULL )
	{
		printf( "Error! Source file could not be open!
		" );
		system( "pause" );
		return 0;
	}


	for( int i = 0; i < parts; i++ )
	{
		sprintf( aux, "%s%s-%d", name, ext, i );
		printf( "Generating %s file! -- ", aux );

		if( (destination = fopen(aux, "wb")) == NULL )
		{
			printf( "Error! Destination file %s could not be created!", aux );
			system( "pause" );
			return 0;
		}

		char car = getc( source );
		for( int j = 0; j < size/parts && !feof(source); j++ )
		{
			putc( car, destination );
			car = getc( source );
		}

		printf( "OK!
		" );
		fclose( destination );
	}

	fclose( source );
	system( "pause" );
	return 0;
}

int fileSize( char File[] )
{
	FILE *f;

	if( (f = fopen(File,"rb")) == NULL )
	{
		printf( "Error! Source file could not be open!" );
		system( "pause" );
		return 0;
	}

	fseek( f, 0,SEEK_END );
	int sz = ftell(f);
	fclose( f );

	return sz;
}

void fileNameExt( char Name[], char Ext[], char File[] )
{
	char *aux;
	if( (aux = strrchr(File, '')) == NULL )
	{
		if( (aux = strrchr( File, '/' )) == NULL)
		{
			aux = File;
		}
	}

	if( aux != File )
	{
		aux++;
	}

	for( int i = 0; *aux != '.'; i++, aux++ )
	{
		Name[i] = *aux;
		Name[i+1] = '\0;';
	}

	for( int i = 0; *aux != '\0;'; i++, aux++ )
	{
		Ext[i] = *aux;
		Ext[i+1] = '\0';
	}
}


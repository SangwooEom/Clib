#include "csv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int set_dquote_flag(const int prev_dequote_flag, const char buffer);

/**
 * 작성자: 엄상우
 * 기능: CSV 파일을 읽는 함수
 * 매개변수 csv_path는 읽어야 할 csv 파일의 경로를 나타냅니다.
 */
CSV* read_csv(const char* csv_path) {
	CSV* csv = malloc(sizeof(CSV));
	char*** data;

	// 1단계: CSV에 저장할 레코드의 수를 계산하여 메모리를 할당한다.

	// 1-1 단계: 파일 포인터를 얻는다.
	FILE *fp = fopen(csv_path, "r");

	// 1-2 단계: 파일 맨 끝에 있는 개행 문자들을 파일이 읽는 동아 인식하지 못하게 처리를 한다.

	// 1-2-1 단계: 파일의 전체 바이트의 개수를 구한다.
	fseek(fp, 0, SEEK_END);
	int all_bytes = ftell(fp);
	fseek(fp, 0, 0);

	char* content = malloc(sizeof(char) * all_bytes);
	fread(content, all_bytes, 1, fp);

	// 1-2-2 단계: 파일의 맨 끝에 개행 문자를 오직 한개만 남기기 위해서 파일 끝에서 개행 문자를 하나씩 지우는 처리를 한다.
	int last_index = all_bytes;
	char buffer;

	do {
		last_index--;
		fseek(fp, 0, last_index);
		fread(&buffer, sizeof(buffer), 1, fp);
	} while (buffer == '\n');

	// 1-2-4 단계: 파일의 커서 위치를 파일의 시작 위치로 변경합니다.
	fseek(fp, 0, 0);

	// 1-3 단계: 파일에 있는 모든 내용을 1byte씩 읽어서 처리를 한다.
	int numOfLines = 0; // CSV 파일 안에 있는 레코드의 개수

	int dquote_flag = 0; // 해당 바이트가 쌍따옴표 안에 있는지 아닌지를 알 수 있는 플래그

	for (int i = 0; i <= last_index; i++) {
		// char buffer;
		// fread(&buffer, sizeof(char), 1, fp);
		char buffer = content[i];

		if (dquote_flag == 0 && buffer == '\"') dquote_flag = 1;
		else if (dquote_flag == 1 && buffer == '\"') dquote_flag = 0;

		if (dquote_flag == 0 && buffer == '\n') numOfLines++;
	}

	// 1-4단계: 구한 record의 개수를 이용해서 csv의 변수에 메모리를 동적으로 할당한다(할당 해제 필요 없음)
	data = malloc(sizeof(char**) * numOfLines);

	// 2단계: 첫 번째 record에 있는 필드의 개수를 계산하여 메모리를 할당한다.
	int* numOfFieldsMap = malloc(sizeof(int) * numOfLines);

	fseek(fp, 0, SEEK_SET);
	dquote_flag = 0;

	int numOfFields = 1;
	int lineNumber = 0;

	for (int i = 0; i <= last_index; i++) {
		// char buffer;
		// fread(&buffer, sizeof(char), 1, fp);
		char buffer = content[i];

		if (dquote_flag == 0 && buffer == '\"') dquote_flag = 1;
		else if (dquote_flag == 1 && buffer == '\"') dquote_flag = 0;

		if (dquote_flag == 0 && buffer == ',') numOfFields++;

		if (dquote_flag == 0 && buffer == '\n') {
			data[lineNumber] = malloc(sizeof(char*) * numOfFields);
			numOfFieldsMap[lineNumber] = numOfFields;
			numOfFields = 1;
			lineNumber++;
		}
	}

	// 3단계: 각 필드의 크기를 정확히 계산하여 메모리를 할당한다.
	fseek(fp, 0, SEEK_SET);

	dquote_flag = 0;
	lineNumber = 0;

	int fieldNumber = 0;
	int strLength = 0;

	for (int i = 0; i <= last_index; i++) {
		// char buffer;
		// fread(&buffer, sizeof(char), 1, fp);
		char buffer = content[i];

		dquote_flag = set_dquote_flag(dquote_flag, buffer);

		if (dquote_flag == 0 && buffer == ',') {
			data[lineNumber][fieldNumber] = malloc(sizeof(char) * (strLength + 1));
			memset(data[lineNumber][fieldNumber], 0, strLength + 1);
			fieldNumber++;
			strLength = 0;
		} else if (dquote_flag == 0 && buffer == '\n') {
			data[lineNumber][fieldNumber] = malloc(sizeof(char) * (strLength + 1));
			memset(data[lineNumber][fieldNumber], 0, strLength + 1);
			lineNumber++;
			fieldNumber = 0;
			strLength = 0;
		} else {
			strLength++;
		}
	}

	// 4단계: 각 필드에 있는 데이터를 추출하여 저장한다.
	fseek(fp, 0, SEEK_SET);

	dquote_flag = 0;
	lineNumber = 0;
	fieldNumber = 0;

	int index = 0;

	for (int i = 0; i <= last_index; i++) {
		// char buffer;
		// fread(&buffer, sizeof(char), 1, fp);
		char buffer = content[i];

		dquote_flag = set_dquote_flag(dquote_flag, buffer);

		if (dquote_flag == 0 && buffer == '\n') {
			lineNumber++;
			fieldNumber = 0;
			index = 0;
		} else if (dquote_flag == 0 && buffer == ',') {
			fieldNumber++;
			index = 0;
		} else {
			data[lineNumber][fieldNumber][index] = buffer;
			index++;
		}
	}

	csv->data = data;
	csv->numOfFieldsMap = numOfFieldsMap;
	csv->numOfLines = numOfLines;

	free(content);
	fclose(fp);

	return csv;
}

void write_csv(CSV* csv, const char* csv_path) {
	FILE *fp = fopen(csv_path, "w");

	for (int i = 0; i < csv->numOfLines; i++) {
		char** record = csv->data[i];

		for (int j = 0; j < csv->numOfFieldsMap[i]; j++) {
			if (j < (csv->numOfFieldsMap)[i] - 1) {
				fprintf(fp, "%s,", record[j]);
			} else {
				fprintf(fp, "%s\n", record[j]);
			}
		}
	}

	fclose(fp);
}

char* substring(const char* str, int begin, int end) {
	int cnt = 0;
	int size = (end - begin) + 1;
	char* substr = malloc(sizeof(char) * size);

	memset(substr, 0, size);

	for (int i = begin; i < end; i++, cnt++) {
		substr[cnt] = str[i];
	}

	return substr;
}

int set_dquote_flag(const int prev_dequote_flag, const char buffer) {
	if (prev_dequote_flag == 0 && buffer == '\"') return 0;
	else if (prev_dequote_flag == 1 && buffer == '\"') return 1;

	return prev_dequote_flag;
}
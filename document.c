/*Alex Pollock
  UID: 115332645
  ID: apolloc1
*/
#include "document.h"
#include <stdio.h>
#include <string.h>
#define DOCUMENT_H
#define MAX_PARAGRAPH_LINES 20
#define MAX_PARAGRAPHS 15
#define MAX_STR_SIZE 80 
#define HIGHLIGHT_START_STR "["
#define HIGHLIGHT_END_STR "]"
#define SUCCESS 0
#define FAILURE -1

/*Initializes document to be empty*/
int init_document(Document *doc, const char *name){
  int name_length = 0;
  if(doc == NULL){
    return FAILURE;
  }
  if(name == NULL){
    return FAILURE;
  }
  name_length = strlen(name);
  if(name_length > MAX_STR_SIZE){
    return FAILURE;
  }
  
  strcpy(doc->name, name);
  doc->number_of_paragraphs = 0;
  return SUCCESS;
}

/*Sets the number of paragraphs to 0.*/
int reset_document(Document *doc){
  if(doc == NULL){
    return FAILURE;
  }
  doc->number_of_paragraphs = 0;
  return SUCCESS;
}

/*Prints document name, number of paragraphs, and paragraph information.*/
int print_document(Document *doc){
  int num_of_paragraphs = 1, i = 0, j = 0;
  char doc_name[MAX_STR_SIZE + 1];
  if(doc == NULL){
    return FAILURE;
  }
  strcpy(doc_name, doc->name);
  num_of_paragraphs = doc->number_of_paragraphs;
    
  printf("Document name: \"%s\"\n", doc_name);
  printf("Number of Paragraphs: %d\n", num_of_paragraphs);
  for(i = 0; i < doc->number_of_paragraphs; i++){
    for(j = 0; j < doc->paragraphs[i].number_of_lines; j++){
      printf("%s\n", doc->paragraphs[i].lines[j]);
    }
  }
    
  return SUCCESS;
}

/*Add a paragraph after the given paragraph number.*/
int add_paragraph_after(Document *doc, int paragraph_number){
  Paragraph p = {0};
  int i = 0, paragraphs = 0;
  
  if(doc == NULL){
    return FAILURE;
  }
  if(paragraph_number >= MAX_PARAGRAPHS || doc->number_of_paragraphs >=
     MAX_PARAGRAPHS){
    return FAILURE;
  }
  if(paragraph_number > doc->number_of_paragraphs){
    return FAILURE;
  }
  paragraphs = doc->number_of_paragraphs;
  if(paragraph_number == 0){
    for(i = paragraphs - 1; i >= 0; i--){
      doc->paragraphs[i + 1] = doc->paragraphs[i];
    }
  }
  doc->number_of_paragraphs++;
  doc->paragraphs[paragraph_number] = p;
  
  return SUCCESS;
}

/*Add a new line after the given line number.*/
int add_line_after(Document *doc, int paragraph_number, int line_number,
		   const char *new_line){
  int lines = 0, i = 0, str_len = 0;
  char line[MAX_STR_SIZE + 1];
  if(doc == NULL || new_line == NULL){
    return FAILURE;
  }
  if(paragraph_number > doc->number_of_paragraphs){
    return FAILURE;
  }
  if(doc->paragraphs[paragraph_number - 1].number_of_lines ==
     MAX_PARAGRAPH_LINES){
    return FAILURE;
  }
  if(line_number > doc->paragraphs[paragraph_number - 1].number_of_lines){
    return FAILURE;
  }

  str_len = strlen(new_line);
  for(i = 0; i < str_len; i++){
    line[i] = new_line[i];
  }
  line[i] = '\0';
  lines = doc->paragraphs[paragraph_number - 1].number_of_lines;
  if(line_number == 0){
    for(i = lines - 1; i >= 0; i--){
      doc->paragraphs[paragraph_number - 1].lines[i + 1][0] =
	doc->paragraphs[paragraph_number - 1].lines[i][0];
    }
  }
  doc->paragraphs[paragraph_number - 1].number_of_lines++;
  strcpy(doc->paragraphs[paragraph_number - 1].lines[line_number], line);
  
  
  return SUCCESS;
}

/*Returns the number of lines in a given paragraph.*/
int get_number_lines_paragraph(Document *doc, int paragraph_number,
			       int *number_of_lines){
  if(doc == NULL){
    return FAILURE;
  }
  if(number_of_lines == NULL){
    return FAILURE;
  }
  if(paragraph_number > doc->number_of_paragraphs){
    return FAILURE;
  }
  *number_of_lines = doc->paragraphs[paragraph_number].number_of_lines;
  return *number_of_lines;
}

/*Adds a line to the given paragraph.*/
int append_line(Document *doc, int paragraph_number, const char *new_line){
  char line[MAX_STR_SIZE + 1];
  int i = 0, str_len = 0, num_lines = 0;
  if(doc == NULL){
    return FAILURE;
  }
  if(paragraph_number > doc->number_of_paragraphs){
    return FAILURE;
  }
  if(doc->paragraphs[paragraph_number - 1].number_of_lines ==
     MAX_PARAGRAPH_LINES){
    return FAILURE;
  }
  if(new_line == NULL){
    return FAILURE;
  }
  str_len = strlen(new_line);
  for(i = 0; i < str_len; i++){
    line[i] = new_line[i];
  }
  line[i] = '\0';
  num_lines = doc->paragraphs[paragraph_number - 1].number_of_lines;
  strcpy(doc->paragraphs[paragraph_number - 1].lines[num_lines], line);
  doc->paragraphs[paragraph_number - 1].number_of_lines++;

  return SUCCESS;
}

/*Removes the given line from the given paragraph.*/
int remove_line(Document *doc, int paragraph_number, int line_number){
  int i = 0, n = 0, number_lines = 0;
  if(doc == NULL){
    return FAILURE;
  }
  if(paragraph_number > doc->number_of_paragraphs){
    return FAILURE;
  }
  if(line_number > doc->paragraphs[paragraph_number - 1].number_of_lines){
    return FAILURE;
  }
  number_lines = doc->paragraphs[paragraph_number - 1].number_of_lines;
  n = doc->paragraphs[paragraph_number - 1].number_of_lines;
  for(i = line_number; i < n; i++){
    strcpy(doc->paragraphs[paragraph_number - 1].lines[i - 1],
	   doc->paragraphs[paragraph_number - 1].lines[i]);
  }
  *doc->paragraphs[paragraph_number - 1].lines[number_lines - 1] = '\0';
  return SUCCESS;
}

/*Adds the first number of lines from the data array to the document.*/
int load_document(Document *doc, char data[][MAX_STR_SIZE + 1], int data_lines){
  int i = 0, j = 0, number = 0;
  if(doc == NULL){
    return FAILURE;
  }
  if(data == NULL){
    return FAILURE;
  }
  if(data_lines == 0){
    return FAILURE;
  }
  strcpy(doc->paragraphs[number].lines[0], data[0]);
  doc->number_of_paragraphs++;
  for(i = 0; i < data_lines; i++){
    if(data[i][0] == '\0'){
      number++;
      doc->number_of_paragraphs++;
      j = 0;
    }
    strcpy(doc->paragraphs[number].lines[j], data[i]);
    doc->paragraphs[number].number_of_lines++;
    j++;
  }
  
  return SUCCESS;
}

/*Replaces the target string in the doucment with the replacement string.*/
int replace_text(Document *doc, const char *target, const char *replacement){
  int i = 0, j = 0, line_length = 0, match_length = 0, target_length = 0,
    trail_length = 0;
  char *match;
  char *lead, *trail;
  char leading[MAX_STR_SIZE + 1], trailing[MAX_STR_SIZE + 1];
  
  if(doc == NULL || target == NULL || replacement == NULL){
    return FAILURE;
  }
  target_length = strlen(target);
  for(i = 0; i < doc->number_of_paragraphs; i++){
    for(j = 0; j < doc->paragraphs[i].number_of_lines; j++){
      match = strstr(doc->paragraphs[i].lines[j], target);
      if(match){
	match_length = strlen(match);
	line_length = strlen(doc->paragraphs[i].lines[j]);
	strcpy(leading, doc->paragraphs[i].lines[j]);
	lead = &leading[0];
	lead[line_length - match_length] = '\0';
	strcpy(trailing, doc->paragraphs[i].lines[j]);
	trail = &trailing[line_length - match_length + target_length];
	trail_length = strlen(trail);
	trail[trail_length] = '\0';
	strcat(lead, replacement);
	strcat(lead, trail);
	strcpy(doc->paragraphs[i].lines[j], lead);
      }
    }
  }
  return SUCCESS;
}

/*Surrounds the target string with [ and ].*/
int highlight_text(Document *doc, const char *target){
  int i = 0, j = 0, target_length = 0, line_length = 0, match_length = 0,
    trail_length = 0;
  char new_target[MAX_STR_SIZE + 1] = {0}, *match, *lead, *trail;
  char leading[MAX_STR_SIZE + 1], trailing[MAX_STR_SIZE + 1];
  if(doc == NULL || target == NULL){
    return FAILURE;
  }
  new_target[MAX_STR_SIZE + 1] = '\0';
  strcpy(new_target, HIGHLIGHT_START_STR);
  strcat(new_target, target);
  strcat(new_target, HIGHLIGHT_END_STR);
  target_length = strlen(new_target);
  for(i = 0; i < doc->number_of_paragraphs; i++){
    for(j = 0; j < doc->paragraphs[i].number_of_lines; j++){
      match = strstr(doc->paragraphs[i].lines[j], target);
      line_length = strlen(doc->paragraphs[i].lines[j]);
      if(match){
	match_length = strlen(match);
	strcpy(leading, doc->paragraphs[i].lines[j]);
	lead = &leading[0];
	lead[line_length - match_length] = '\0';
	strcpy(trailing, doc->paragraphs[i].lines[j]);
	trail = &trailing[line_length - match_length + target_length - 2];
	trail_length = strlen(trail);
	trail[trail_length] = '\0';
	strcat(lead, new_target);
	strcat(lead, trail);
	strcpy(doc->paragraphs[i].lines[j], lead);
      }
    }
  }

  return SUCCESS;
}

/*Removes the target string from the document everywhere it appears.*/
int remove_text(Document *doc, const char *target){
  int i = 0, j = 0, target_length = 0;
  char *match;
  
  if(doc == NULL || target == NULL){
    return FAILURE;
  }
  
  target_length = strlen(target);
  for(i = 0; i < doc->number_of_paragraphs; i++){
    for(j = 0; j < doc->paragraphs[i].number_of_lines; j++){
      char *original = doc->paragraphs[i].lines[j];
      char lead[MAX_STR_SIZE] = "";
      do{
	match = strstr(original, target);
	if(match){
	  char trail[MAX_STR_SIZE];
	  int index = (match - original) / sizeof(original[0]);
	  strncpy(trail, original, index);
	  trail[index] = '\0';
	  original += index + target_length;
	  strcat(lead, trail);
	}
      } while(match != NULL);
      strcat(lead, original);
      strcpy(doc->paragraphs[i].lines[j], lead);
    }
  }
  return SUCCESS;
}

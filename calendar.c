#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "event.h"
#include "calendar.h"
#include "my_memory_checker_216.h"

/*Initializes a calendar structure by setting the name to the name parameter,
  setting the number of days to the day parameter, using the comp_func, using
  the free_info_func, and provide access to a new calendar structure through the
  out parameter*/
int init_calendar(const char *name, int days,
		  int (*comp_func) (const void *ptr1, const void *ptr2),
		  void (*free_info_func) (void *ptr), Calendar ** calendar){
  int length, i;
  char read[30];

  /*Fails if calendar null, name null, or days less than 1*/
  if(!(calendar) || !(name) || days < 1){
    return FAILURE;
  }

  /*Allocate memory for calendar structure*/
  *calendar = malloc(sizeof(Calendar));
  if(!(*calendar)){
    return FAILURE;
  }
  length = strlen(name);
  (*calendar)->name = malloc(length + 3);
  if(!((*calendar)->name)){
    return FAILURE;
  }
  strcpy(read, name);
  for(i = 0; i < length; i++){
    read[i + 1] = name[i];
  }
  read[0] = '\"';
  read[i + 1] = '\"';
  read[i + 2] = '\0';
  strcpy((*calendar)->name, read);

  /*Allocate memory for events*/
  (*calendar)->events = calloc(days, sizeof(Event *));
  if(!(*calendar)->events){
    return FAILURE;
  }
  (*calendar)->comp_func = comp_func;
  (*calendar)->free_info_func = free_info_func;

  (*calendar)->days = days;
  (*calendar)->total_events = 0;
  
  return SUCCESS;
}

/*Prints calendar name, days, and total number of events to output stream as 
  long as print_all is true. Then prints information about each event.*/
int print_calendar(Calendar *calendar, FILE *output_stream, int print_all){
  int i = 0;
  Event *curr;

  /*Fails if calendar or output stream null*/
  if(!(calendar) || !(output_stream)){
    return FAILURE;
  }
  if(print_all == 1){
    fprintf(output_stream, "Calendar's Name: %s\n", calendar->name);
    fprintf(output_stream, "Days: %d\n", calendar->days);
    fprintf(output_stream, "Total Events: %d\n", calendar->total_events);
  }
  fprintf(output_stream, "\n**** Events ****\n");
  
  for(i = 0; i < calendar->days; i++){
    if(calendar->total_events != 0){
      printf("Day %d\n", i + 1);
    }
    curr = calendar->events[i];
    while(curr){
      if(curr->name){
      printf("Event's Name: \"%s\", Start_time: %d, Duration: %d\n", curr->name, curr->start_time, curr->duration_minutes);
      }
      curr = curr->next;
    }
  }
  
  return SUCCESS;
}

/*Add specified event to the list associated with the given day. Event is added
  in increasing order.*/
int add_event(Calendar *calendar, const char *name, int start_time,
	      int duration_minutes, void *info, int day){
  Event *curr, *new, *next, *event_ptr;
  int (*pf)(const void *, const void *);
  int length, add = 0;
  
  /*Fails is calendar null, name null, minutes less than or equal to 0, start 
    time is less than 0 or greater than 2400, day less than 1, or greater than
    calendar days*/
  if(!calendar || !name || duration_minutes <= 0 || start_time < 0 ||
     start_time > 2400 || day < 1 || day > calendar->days){
    return FAILURE;
  }
 
  /*Event already exists*/
  event_ptr = calendar->events[day - 1];
  while(event_ptr != NULL){
    if(strcmp(event_ptr->name, name) == 0){
      return FAILURE;
    }
    event_ptr = event_ptr->next;
  }

  curr = calendar->events[day - 1];
  pf = calendar->comp_func;

  /*Create new event*/
  new = malloc(sizeof(Event));
  if(!new){
    return FAILURE;
  }
  length = strlen(name);
  new->name = malloc(length + 1);
  if(!new->name){
    return FAILURE;
  }
  strcpy(new->name, name);
  new->start_time = start_time;
  new->duration_minutes = duration_minutes;
  new->info = info;
  new->next = NULL;

  
  /*Add event in increasing sorted order*/
  if(!curr){
    calendar->events[day - 1] = new;
    add = 1;
  }
  else if(add == 0 && pf(new, curr) < 0){
    new->next = curr;
    calendar->events[day - 1] = new;
    add = 1;
  }
  while(curr && !add){
    next = curr->next;
    if(pf(new, curr) > 0 && (!next || pf(new, next) < 0)){
      new->next = next;
      curr->next = new;
      add = 1;
    }
    curr = curr->next;
  }
  calendar->total_events++;
  return SUCCESS;
}

/*Returns pointer to the event if found given the specified name.*/
int find_event(Calendar *calendar, const char *name, Event **event){
  int i, days;

  /*Fails if calendar or name parameters are null*/
  if(!(calendar) || !(name)){
    return FAILURE;
  }
  
  days = calendar->days;
  for(i = 1; i <= days; i++){
    if(find_event_in_day(calendar, name, i, event) == SUCCESS){
      return SUCCESS;
    }
  }
  
  return FAILURE;
}

/*Returns a pointer to event if found given the specified name and day*/
int find_event_in_day(Calendar *calendar, const char *name, int day,
		      Event **event){
  Event *event_ptr;

  /*Fail if calendar is null, name is null, day is less than 1, or day parameter
    greater than calendar days*/
  if(!calendar || !name || day < 1 || day > calendar->days){
    return FAILURE;
  }
  
  event_ptr = calendar->events[day - 1];
  while(event_ptr){
    if(strcmp(event_ptr->name, name) == 0){
      if(event){
	*event = event_ptr;
      }
      return SUCCESS;
    }
    event_ptr = event_ptr->next;
  }  
  return FAILURE;
}

/*Removes an event from list given the specified name parameter*/
int remove_event(Calendar *calendar, const char *name){
  void(*free_info) (void *ptr);
  Event *curr, *prev, **event_ptr;
  int i, days;

  /*Fails if calendar or name parameters are null*/
  if(!(calendar) || !(name)){
    return FAILURE;
  }

  prev = NULL;
  days = calendar->days;
  event_ptr = malloc(sizeof(Event *));

  for(i = 0; i < days; i++){
    curr = calendar->events[i];
    free_info = calendar->free_info_func;
    /*While there are events in a day*/
    prev = curr;
    while(curr){
      if(find_event_in_day(calendar, name, i + 1, event_ptr)  == SUCCESS){
	/*only 1 event in day*/
	if(!prev->next){
	  free(calendar->events[i]->name);
	  calendar->events[i]->name = NULL;
	  free(calendar->events[i]);
	  calendar->events[i] = NULL;
	  calendar->days--;
	  calendar->total_events--;
	  free(event_ptr);
	    return SUCCESS;
	}

	/*Finds event in the day*/
	if(strcmp(curr->name, name) == 0){
	  prev->next = curr->next;
	  calendar->events[i] = prev;
	  free((*event_ptr)->name);
	  free(*event_ptr);
	  free(event_ptr);
	  calendar->total_events--;
	  return SUCCESS;
	}
      }
      /*Moves to next event in day*/
      curr = curr->next;
    }
  }
  return FAILURE;
}


/*Returns info pointer associated with specified name*/
void *get_event_info(Calendar *calendar, const char *name){
  Event *event;
  
  if(find_event(calendar, name, &event) == SUCCESS){
    return (*event).info;
  }
  return NULL;
}

/*Removes all event list associated with calendar, setting lists to empty. */
int clear_calendar(Calendar *calendar){
  void(*free_info) (void *ptr);
  int i;

  /*Fails is calendar is null*/
  if(!calendar){
    return FAILURE;
  }

  free_info = calendar->free_info_func;
  for(i = 0; i < calendar->days; i++){
    if(free_info && calendar->events[i]->info){
      calendar->events[i]->info = NULL;
      free_info(calendar->events[i]->info);
    } else {
      clear_day(calendar, i + 1);
      calendar->events[i] = NULL;
    }
  }
 
  calendar->total_events = 0;

  return SUCCESS;
}

/*Removes all events for a specified day setting event list to empty*/
int clear_day(Calendar *calendar, int day){
  void (*free_info) (void *ptr);
  Event *curr, *tmp;
  void *info;

  /*Fails is calendar is null, day is less than 1 or greater than number of days
    in the calendar*/
  if(!calendar || day < 1 || day > calendar->days){
    return FAILURE;
  }
  
  free_info = calendar->free_info_func;
  curr = calendar->events[day - 1];
  while(curr != NULL){
    tmp = curr->next;
    free(curr->name);
    info = curr->info;
    if(info && free_info){
      free_info(info);
    }
    free(curr);
    curr = tmp;
    calendar->total_events--;
  }
  calendar->events[day - 1] = NULL;
  return SUCCESS;
}

/*Return memory that was allocated for the calendar*/
int destroy_calendar(Calendar *calendar){
  int size, i;

  /*Fails if calendar is null*/
  if(!calendar){
    return FAILURE;
  }

  size = calendar->days;
  for(i = 1; i <= size; i++){
    clear_day(calendar, i);
  }
  
  free(calendar->events);
  free(calendar->name);
  free(calendar);
 
  
  return SUCCESS;
}

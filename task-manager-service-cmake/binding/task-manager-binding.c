#include <proc/readproc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <proc/sysinfo.h>
#include <math.h>
#include <unistd.h>
#include <json-c/json.h>

#define AFB_BINDING_VERSION 2
#include <afb/afb-binding.h>

void get_process_list(struct afb_req request);
struct pstat* fill_pstat(proc_t *proc_info);
struct cpu_percentage* cpu_calculate(struct pstat*, struct pstat*);
struct process_container* process_container_constr(char* process_name, int euid, struct pstat *pstat_values);
static const struct afb_binding_interface *interface;
static struct afb_event event;

struct pstat {
	long long unsigned int utime_ticks;
	long long unsigned int cutime_ticks;
	long long unsigned int stime_ticks;
	long long unsigned int cstime_ticks;
	long unsigned int cpu_total_time;
};

struct cpu_percentage {
	double ucpu_usage;
	double scpu_usage;
};

struct process_container {
	char *process_name;
	// int tid;
	int euid;
	struct pstat *pstat_values;
};

void get_process_list(struct afb_req request){

	int seconds = 1, page_size;
	page_size = getpagesize();
	struct pstat *last_pstat_values, *now_pstat_values;
	struct cpu_percentage *cpu_usage;
	struct process_container *process_obj, *elem = NULL;
	struct process_container* object_container[65535]; // array holding process objects
	struct json_object *ret_json, *json_array, *json_obj;
	ret_json = json_object_new_object();
	json_array = json_object_new_array();
	char state_str[2] = "\0";

	PROCTAB* proc = openproc(PROC_FILLMEM | PROC_FILLSTAT);
	if (!proc){
		AFB_REQ_ERROR(request, "Unable to open /proc!");
		afb_req_fail_f(request, "Failed", "Error processing arguments.");
	}
	
	proc_t* proc_info;
	while ((proc_info = readproc(proc, NULL)) != NULL) {
		last_pstat_values = fill_pstat(proc_info);
		process_obj = process_container_constr(proc_info->cmd, proc_info->euid, last_pstat_values);
		object_container[proc_info->tid] = process_obj;
		freeproc(proc_info);
	}

	sleep(seconds);

	proc = openproc(PROC_FILLMEM | PROC_FILLSTAT);
	if (!proc){
		AFB_REQ_ERROR(request, "Unable to open /proc!");
		afb_req_fail_f(request, "Failed", "Error processing arguments.");
	}

	proc_t* proc_info2;
	while ((proc_info2 = readproc(proc, NULL)) != NULL) {
		now_pstat_values = fill_pstat(proc_info2);
		elem = object_container[proc_info2->tid];
		if(elem){
			cpu_usage = cpu_calculate(elem->pstat_values, now_pstat_values);
			json_obj = json_object_new_object();
			json_object_object_add(json_obj, "cmd", json_object_new_string(proc_info2->cmd));
			json_object_object_add(json_obj, "tid", json_object_new_int(proc_info2->tid));
			json_object_object_add(json_obj, "euid", json_object_new_int(proc_info2->euid));
			json_object_object_add(json_obj, "scpu", json_object_new_double(cpu_usage->scpu_usage));
			json_object_object_add(json_obj, "ucpu", json_object_new_double(cpu_usage->ucpu_usage));
			json_object_object_add(json_obj, "resident_mem", json_object_new_double((proc_info2->resident * page_size)/ pow(1024, 2)));
			state_str[0] = proc_info2->state;
			json_object_object_add(json_obj, "state", json_object_new_string(state_str));
			json_object_array_add(json_array, json_obj);
		}
		freeproc(proc_info2);
	}
	json_object_object_add(ret_json, "processes", json_array);
	afb_req_success(request, ret_json, NULL);

	closeproc(proc);
	// printf ("The json object created: %s\n", json_object_to_json_string(ret_json));
}

struct pstat* fill_pstat(proc_t *proc_info){

	long unsigned int cpu_total_time;
	long unsigned int cpu_time[10];
	struct pstat *pstat_values = malloc(sizeof(struct pstat));

	pstat_values->utime_ticks = proc_info->utime;
	pstat_values->cutime_ticks = proc_info->cutime;
	pstat_values->stime_ticks = proc_info->stime;
	pstat_values->cstime_ticks = proc_info->cstime;

	FILE *fstat = fopen("/proc/stat", "r");
	if (fstat == NULL) {
		perror("FOPEN ERROR ");
		fclose(fstat);
	}

	memset(cpu_time, 0, sizeof(cpu_time));
	if (fscanf(fstat, "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
				&cpu_time[0], &cpu_time[1], &cpu_time[2], &cpu_time[3],
				&cpu_time[4], &cpu_time[5], &cpu_time[6], &cpu_time[7],
				&cpu_time[8], &cpu_time[9]) == EOF) {
		fclose(fstat);
	}

	fclose(fstat);

	/*
	 * Returns total CPU time. It is a sum of user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice
	 *
	 *
	 */
	for(int i = 0; i < 10; i++)
		pstat_values->cpu_total_time += cpu_time[i];  

	return pstat_values;
}

struct cpu_percentage* cpu_calculate(struct pstat *last_pstat_values, struct pstat *now_pstat_values){

	long unsigned int total_time_diff = now_pstat_values->cpu_total_time - last_pstat_values->cpu_total_time;
	
	struct cpu_percentage *cpu_values = malloc(sizeof(struct cpu_percentage));

	cpu_values->ucpu_usage = 100 * (((now_pstat_values->utime_ticks + now_pstat_values->cutime_ticks)
	- (last_pstat_values->utime_ticks + last_pstat_values->cutime_ticks)) / (double) total_time_diff);

	cpu_values->scpu_usage = 100 * (((now_pstat_values->stime_ticks + now_pstat_values->cstime_ticks)
	- (last_pstat_values->stime_ticks + last_pstat_values->cstime_ticks)) / (double) total_time_diff);

	return cpu_values;
}

struct process_container* process_container_constr(char* process_name, int euid, struct pstat *pstat_values) {

	struct process_container *r = malloc(sizeof(struct process_container));
	r->process_name = process_name;
	r->euid = euid;
	r->pstat_values = pstat_values;

	return r;
}

static const struct afb_verb_v2 _afb_verbs_v2_taskmanager[] = {
	{
		.verb = "get_process_list",
		.callback = get_process_list,
		.auth = NULL,
		.info = "Get an array of all processes currently running on the system",
		.session = AFB_SESSION_NONE_V2
	}
};


const struct afb_binding_v2 afbBindingV2 = {
    .api = "taskmanager",
    .specification = NULL,
    .info = "Task Manager service",
    .verbs = _afb_verbs_v2_taskmanager,
    .noconcurrency = 0
};

/*UI issues a web socket request to the binding.
  which means calling the callback.
  Token for afb-client-demo API is hello/1234 or system dunit file.	
*/
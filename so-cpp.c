#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct Hash_table {
	char *data;
	char *key;
} hash_table;

void *add_to_hash_table(struct Hash_table *hash_list, char *data,
	char *key, int *index)
{
	hash_list[*index].data = (char *)calloc(strlen(data) + 1, sizeof(char));
	if (hash_list[*index].data == NULL)
		return NULL;
	hash_list[*index].key = (char *)calloc(strlen(key) + 1, sizeof(char));
	if (hash_list[*index].key == NULL) {
		free(hash_list[*index].data);
		return NULL;
	}
	strcpy(hash_list[*index].data, data);
	strcpy(hash_list[*index].key, key);
	*index = *index + 1;
	return hash_list;
}

char *find_item(struct Hash_table *hash_list, char *key, int *index)
{
	int i;

	for (i = 0; i < *index; i++) {

		if (!strcmp(hash_list[i].data, key))
			return hash_list[i].key;
	}
	return NULL;
}

int check_if_exists(struct Hash_table *hash_list, char *key, int *index)
{
	int i;

	for (i = 0; i < *index; i++) {

		if (!strcmp(hash_list[i].data, key))
			return 1;
	}
	return 0;
}

void delete_from_list(struct Hash_table *hash_list, char *key, int *index)
{
	int i, j;

	for (i = 0; i < *index; i++) {

		if (!strcmp(hash_list[i].data, key)) {
			for (j = i; j < *index - 1; j++)
				hash_list[j] = hash_list[j + 1];
			free(hash_list[i].data);
			free(hash_list[i].key);
		}
	}
	*index = *index - 1;
}


void define_func(int hash_list_index, struct Hash_table *hash_list,
	int lines_count, char **lines, char *buffer,
	int def_arg, int undef_after, int if_arg)
{
	int count = hash_list_index - def_arg, k, h, i = 0;
	char *occ;
	char **line = (char **)calloc(100, sizeof(char *));
	char *change_buffer = (char *)calloc(550, sizeof(char));

	if (def_arg == 1 && hash_list_index != 1)
		count = count - 1;

	for (i = 0; i < 100; i++)
		line[i] = (char *)calloc(100, sizeof(char));
	i = 0;
	for (k = 0; k < hash_list_index; k++) {
		for (h = count; h < lines_count; h++) {
			if (undef_after == 0 || h < undef_after) {
				occ = strstr(lines[h], hash_list[k].data);
				if (occ) {
					if (strstr(occ + strlen(hash_list[k].data),
					 hash_list[k].data))
						occ = strstr(occ + strlen(hash_list[k].data),
						 hash_list[k].data);
					strncpy(line[i], lines[h], occ - lines[h]);
					line[i][occ-lines[h] + 1] = '\0';
					strcat(line[i], hash_list[k].key);
					occ += strlen(hash_list[k].data);
					strcat(line[i], occ);
					strcpy(lines[h], line[i]);
					i++;
				}
			}
		}
	}
	for (h = count; h < lines_count; h++) {
		if (undef_after != 0 && h == undef_after)
			continue;
		strcat(change_buffer, lines[h]);
		strcat(change_buffer, "\n");
	}
	strcpy(buffer, change_buffer);
	free(change_buffer);
	for (i = 0; i < 100; i++)
		free(line[i]);
	free(line);
}

void include(char **lines, char *buffer, int lines_count)
{
	char *change_buffer = (char *)calloc(550, sizeof(char));
	int i;

	change_buffer[0] = '\0';
	for (i = 1; i < lines_count; i++) {
		strcat(change_buffer, lines[i]);
		strcat(change_buffer, "\n");
	}
	strcpy(buffer, change_buffer);
	free(change_buffer);
}

void ifdef_func(char **lines, int lines_count, int *hash_list_index,
	int def_arg, char *buffer, struct Hash_table *hash_list)
{
	int count = *hash_list_index - def_arg, i;
	char *change_buffer = (char *)calloc(550, sizeof(char));
	char *data, *key;

	change_buffer[0] = '\0';

	for (i = count; i < lines_count; i++) {
		if (strstr(lines[i], "#ifdef ")) {
			data = strtok(lines[i], "#ifdef ");
			if (check_if_exists(hash_list, data, hash_list_index) == 0) {
				while (strcmp(lines[i], "#endif"))
					i++;
			} else {
				i++;
				if (strstr(lines[i], "#undef")) {
					key = strtok(lines[i], "#undef ");
					delete_from_list(hash_list, key, hash_list_index);
					define_func(*hash_list_index, hash_list, lines_count,
					lines, change_buffer, def_arg - 4, 0, -1);
					break;
				}
			}
		}
		if (strstr(lines[i], "#ifndef")) {
			data = strtok(lines[i], "#ifndef ");
			if (check_if_exists(hash_list, data, hash_list_index) == 0) {
				i++;
				data = strtok(lines[i], "#define ");
				key = strtok(NULL, " ");
				if (add_to_hash_table(hash_list, data, key, hash_list_index) == NULL) {
					free(change_buffer);
					exit(-1);
				}
				define_func(*hash_list_index, hash_list, lines_count,
				lines, change_buffer, def_arg - 2, 0, -1);
				break;
			}
		}
		if (strstr(lines[i], "#endif"))
			i++;
		strcat(change_buffer, lines[i]);
		strcat(change_buffer, "\n");
	}
	strcpy(buffer, change_buffer);
	free(change_buffer);
}

void if_func(char **lines, int lines_count, int hash_list_index, int def_arg,
	int if_arg, char *buffer)
{
	int i, j, count = hash_list_index - def_arg, end_found = 0, elif_found = 0;
	char *change_buffer = (char *)calloc(550, sizeof(char)), *occ;
	char *data;

	change_buffer[0] = '\0';

	for (i = count; i < lines_count; i++) {
		if (i < if_arg) {
			strcat(change_buffer, lines[i]);
			strcat(change_buffer, "\n");
		}
		occ = strstr(lines[i], "#if ");
		if (occ) {
			data = strtok(lines[i], "#if ");
			if (atoi(data) > 0) {
				for (j = i + 1; j < lines_count; j++) {
					if (!strcmp(lines[j], "#else") || !strcmp(lines[j],
						"#elif")) {
						while (strcmp(lines[j], "#endif"))
							j++;
					}
					if (!strcmp(lines[j], "#endif"))
						j++;
					if (strstr(change_buffer, lines[j]) == NULL) {
						strcat(change_buffer, lines[j]);
						strcat(change_buffer, "\n");
					}
				}
			} else if (atoi(data) <= 0) {
				for (j = i + 1; j < lines_count; j++) {
					if (!strcmp(lines[j], "#elif 1")) {
						elif_found = 1;
						j++;
						end_found = 1;
					}
					if (!strcmp(lines[j], "#elif 0")) {
						j++;
						end_found = 0;
					}
					if ((!strcmp(lines[j], "#else"))) {
						end_found = 1;
						j++;
						if (elif_found == 1) {
							while (strcmp(lines[j], "#endif"))
								j++;
						}
					}
					if (!strcmp(lines[j], "#endif")) {
						j++;
						end_found = 1;
						if (strstr(lines[j], "#if"))
							continue;
					}
					if (end_found == 1) {
						strcat(change_buffer, lines[j]);
						strcat(change_buffer, "\n");
					}
				}
			}
		}
	}
	strcpy(buffer, change_buffer);
	free(change_buffer);
}

int main(int argc, char *argv[])
{
	FILE *fd_input, *fd_include, *fd_path;
	FILE *fd_output;
	char line_char = '-';
	char letter_d = 'D';
	char letter_o = 'o';
	char letter_i = 'I';
	int i, j, k;
	int input_file = 0;
	int output_file = 0;
	int def = 0, def_arg = 0, if_arg = -1, cons_def = 1, if_def = 0, include_found = 0;
	int lines_count = 0, ind = 0, dir_found = 0;
	int undef_after = 0, if_n_def = 0;
	int hash_list_index = 0;
	struct Hash_table *hash_list = calloc(20, sizeof(struct Hash_table));
	char *buffer = (char *)calloc(550, sizeof(char));
	char **lines = (char **)calloc(100, sizeof(char *));
	char **lines2 = (char **)calloc(100, sizeof(char *));
	char *change_buffer = (char *)calloc(550, sizeof(char));
	char *buffer2 = (char *)calloc(550, sizeof(char));
	char *path = (char *)calloc(550, sizeof(char));
	char *include_file = (char *)calloc(550, sizeof(char));
	char *dir_path = (char *)calloc(550, sizeof(char));
	char *data, *key;
	char *out_file, *token;
	char *key_and_data;
	char *keys[10];
	char *new_key;
	char *new_data;
	char argv_new[1][10];

	for (i = 0; i < 100; i++)
		lines[i] = (char *)calloc(100, sizeof(char));
	for (i = 0; i < 100; i++)
		lines2[i] = (char *)calloc(100, sizeof(char));

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == line_char) {
			if (argv[i][1] == letter_d) {
				def = 1;
				def_arg = 1;
				if (strlen(argv[i]) < 3 && ((i + 1) < argc)) {
					i++;
				} else {
					k = 0;
					for (j = 2; j < (int)strlen(argv[i]); j++) {
						argv_new[0][k] = argv[i][j];
						k++;
					}
					argv_new[0][k] = '\0';
					strcpy(argv[i], argv_new[0]);
				}
				data = strtok(argv[i], "=");
				key = strtok(NULL, "=");
				if (key == NULL)
					key = "";
				if (add_to_hash_table(hash_list, data, key,
					&hash_list_index) == NULL) {
					free(buffer);
					exit(-1);
				}
			} else if (argv[i][1] == letter_o) {
				output_file = 1;
				if (strlen(argv[i]) < 3 && (i + 1) < argc)
					i++;
				out_file = strtok(argv[i], &letter_o);
				fd_output = fopen(out_file, "w+");
			} else if (argv[i][1] == letter_i && (i + 1) < argc) {
				if (strlen(argv[i]) < 3)
					i++;
				dir_found = 1;
				strcpy(dir_path, argv[i]);

			} else {
				free(buffer);
				exit(-1);
			}
		} else {
			if (input_file == 0) {
				input_file = 1;
				fd_input = fopen(argv[i], "r");
				if (fd_input == NULL) {
					free(buffer);
					exit(-1);
				}
				fread(buffer, 550, 1, fd_input);
				fclose(fd_input);
				strcpy(path, argv[i]);
				strcat(buffer, "\0");
				strcpy(buffer2, buffer);
				token = strtok(buffer2, "\n");
				while (token != NULL) {
					strcpy(lines[lines_count], token);
					strcpy(lines2[lines_count], token);
					lines_count++;
					token = strtok(NULL, "\n");
				}
				for (k = 0; k < lines_count; k++) {
					if (strstr(lines[k], "define") && cons_def == 1 && if_n_def == 0) {
						def = 1;
						key_and_data = strtok(lines2[k], "#define");
						data = strtok(key_and_data, " ");
						key = strtok(NULL, "\n");
						if (key == NULL)
							key = "";
						ind = 0;
						token = strtok(key, " ");
						while (token != NULL) {
							keys[ind] = token;
							ind++;
							token = strtok(NULL, " ");
						}
						new_key = malloc(sizeof(char) * 30);
						new_key[0] = '\0';
						for (j = 0; j < ind; j++) {
							new_data = find_item(hash_list, keys[j], &hash_list_index);
							if (new_data != NULL) {
								keys[j] = new_data;
								strcat(new_key, keys[j]);
								if (j + 1 != ind)
									strcat(new_key, " ");
							} else {
								strcat(new_key, keys[j]);
								if (j + 1 != ind)
									strcat(new_key, " ");
							}
						}
						strcpy(key, new_key);
						free(new_key);
						if (add_to_hash_table(hash_list, data, key,
							&hash_list_index) == NULL) {
							free(buffer2);
							free(buffer);
							exit(-1);
						}
					} else if (strstr(lines[k], "#undef")) {
						cons_def = 0;
						undef_after = k;
						key_and_data = strtok(lines[k], "#undef");
						data = strtok(key_and_data, " ");
					} else if (strstr(lines[k], "#if ") && if_arg == -1) {
						cons_def = 0;
						if_arg = k;
					} else if (strstr(lines[k], "#ifdef")) {
						if_def = 1;
					} else if (strstr(lines[k], "#ifndef")) {
						if_n_def = 1;
						if_def = 1;
					} else if (strstr(lines[k], "#include")) {
						key_and_data = strtok(lines2[k], " ");
						data = strtok(NULL, " ");
						data[strlen(data) + 1] = '\0';
						for (j = 0; j < strlen(data); j++)
							data[j] = data[j + 1];
						data[strlen(data) - 1] = '\0';
						key = strstr(path, "/");
						if (key != NULL)
							key = strstr(key + 1, "/");
						memset(key, '\0', 1);
						strcat(path, "/");
						strcat(path, data);
						fd_include = fopen(path, "r");
						include_found = 1;
						if (include_found == 1 && dir_found == 1)
							strcat(dir_path, key);
						fd_path = fopen(dir_path, "r");
						if (fd_include == NULL && fd_path == NULL)
							exit(-1);
						if (fd_include != NULL) {
							fread(include_file, 550, 1, fd_include);
							include(lines, buffer, lines_count);
							fclose(fd_include);
						}
						if (fd_path != NULL) {
							fread(include_file, 550, 1, fd_path);
							include(lines, buffer, lines_count);
							fclose(fd_path);
						}
					} else {
						cons_def = 0;
					}
				}
				if (def == 1 && if_def == 0) {
					define_func(hash_list_index, hash_list, lines_count,
					lines, buffer, def_arg, undef_after, if_arg);
				}
				if (if_arg != -1) {
					if_func(lines, lines_count, hash_list_index, def_arg,
					 if_arg, buffer);
				}
				if (if_def == 1) {
					ifdef_func(lines, lines_count, &hash_list_index,
					 def_arg, buffer, hash_list);
				}
			}
			if (output_file == 0  && strstr(argv[i], "out")) {
				output_file = 1;
				fd_output = fopen(argv[i], "w+");
				if (fd_output == NULL) {
					free(buffer);
					exit(-1);
				}
			} else if (output_file == 1) {
				free(buffer);
				exit(-1);
			}
		}
	}

	if (input_file == 0) {
		while (fgets(buffer, 550, stdin))
			strcat(buffer2, buffer);
	}
	if (output_file == 0) {
		if (include_found == 1)
			fprintf(stdout, "%s\n", include_file);
		fprintf(stdout, "%s", buffer);
	}
	if (output_file == 1) {
		if (def == 1) {
			define_func(hash_list_index, hash_list, lines_count, lines,
			 buffer, def_arg, undef_after, if_arg);
		}
		if (include_found == 1)
			fprintf(fd_output, "%s\n", include_file);
		fprintf(fd_output, "%s\n", buffer);
		fclose(fd_output);
	}
	for (j = 0; j < hash_list_index; j++) {
		free(hash_list[j].data);
		free(hash_list[j].key);
	}
	for (j = 0; j < 100; j++) {
		free(lines[j]);
		free(lines2[j]);
	}
	free(hash_list);
	free(dir_path);
	free(include_file);
	free(path);
	free(lines2);
	free(lines);
	free(buffer);
	free(buffer2);
	free(change_buffer);

	return 0;
}

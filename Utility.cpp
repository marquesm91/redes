#include "Utility.h"

void ShowResults(struct Info *Infos, int times, const char *protocol, int multi_connections)
{
    int i; /* Iterator */

    /* Print headers on the correct way */
    if (multi_connections == 1)
        printf("\n\t\t\t*** %s %s RESULTS ***\n\n", protocol, "MULTIPLE CONNECTIONS");
    else
        printf("\n\t\t\t*** %s RESULTS ***\n\n", protocol);

    if (UNIT == 1000000)
        puts("FILENAME\t\tDATA PACKAGES(Bytes)\tAVERAGE RTT(ms)\t\tTHROUGHPUT(Mbits/s)");
    else if (UNIT == 1000)
        puts("FILENAME\t\tDATA PACKAGES(Bytes)\tAVERAGE RTT(ms)\t\tTHROUGHPUT(kbits/s)");

    /* Create correct name of file. Could be UDP.txt or TCP.txt*/
    char *file_to_save = (char *) malloc((3 + 4 + 1) * sizeof(char));
    strcpy(file_to_save, protocol);
    strcat(file_to_save, ".txt");
    file_to_save[7] = '\0';

    FILE *output_file = fopen(file_to_save, "w+t");

    for (i = 0; Infos->total_time[i] != '\0'; i++)
    {
        /* Print on screen */
        printf("%d.txt            ", Infos->filenames[i]);
        printf("\t%d                ", Infos->size_packages[i]);
        printf("\t%0.4Lf          ", (static_cast<long double>(Infos->total_time[i]) / 1000000.0) / static_cast<long double>(times));
        printf("\t%0.4Lf\n", Infos->throughput[i] / UNIT);

        /* Write on file */
        fprintf(output_file, "%d ", Infos->filenames[i]);
        fprintf(output_file, "%d ", Infos->size_packages[i]);
        fprintf(output_file, "%0.4Lf ", (static_cast<long double>(Infos->total_time[i]) / 1000000.0) / static_cast<long double>(times));
        fprintf(output_file, "%0.4Lf\n", Infos->throughput[i] / UNIT);
    }

    fclose(output_file);
    free(file_to_save);
}

void *GetAddr(struct sockaddr *sa)
{
    /* Get the correct address. IPv6 or IPv4 */
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in *)sa)->sin_addr);
    else
        return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

unsigned long long GetTimestamp(struct timespec &after, struct timespec &before)
{
    /* Get timestamp in nanoseconds (best resolution) */
    unsigned long long elapsed = ((after.tv_sec * (unsigned long long)1000000000) + (after.tv_nsec))
                                 - ((before.tv_sec * (unsigned long long)1000000000) + (before.tv_nsec));

    /* Print timestamp in milliseconds with 4 decimal places*/
    printf("RTT = %0.4Lf ms\n", (static_cast<long double>(elapsed) / 1000000.0));

    return elapsed;
}

long double GetThroughput(unsigned long long total_time, int times, int filename)
{
    /* Get RTT in nanoseconds and then converting to seconds */
    long double rtt_s = ((static_cast<long double>(total_time) / static_cast<long double>(times)) / 1000000000.0);

    /* Return the throughput in bits per second*/
    return ((filename * 8.0) / (rtt_s));
}

int GetPackages(char ***cbuffer, int nbytes, int width)
{
    int total_packages; /* Counter for total packages to be returned */
    int i, j; /* Iterators */ 
    char ch; /* Get the current character of the file to be open */

    /* Get the right space for filename */
    int filename_nbytes = 0;
    for (int aux_nbytes = nbytes; aux_nbytes > 0; aux_nbytes /= 10) filename_nbytes++;
    filename_nbytes += 4; /* add space for ".txt" */
    char *filename = (char *) malloc((filename_nbytes + 11 + 1) * sizeof(char)); /* +11 to include ./Entradas/ */
    char *justname = (char *) malloc((filename_nbytes - 4 + 1) * sizeof(char));

    /* Create the name of file on format ./Entradas/filename.txt */
    strcpy(filename, "./Entradas/");
    snprintf(justname, filename_nbytes - 4 + 1, "%d", nbytes);
    strcat(filename, justname);
    strcat(filename, ".txt");
    filename[filename_nbytes + 11] = '\0';

    /* Open filename */
    FILE *output_file = fopen(filename, "r+t");

    if (output_file != NULL)
    {
        /* Get the total packages on filename*/
        if (width == 0)
            total_packages = 1;
        else if (nbytes / width == 0)
            total_packages = 1;
        else if (nbytes % width > 0)
            total_packages = (nbytes / width) + 1;
        else if (nbytes % width == 0)
            total_packages = (nbytes / width);

        /* Allocate space to get the matrix of packages */
        char **packages = (char **) malloc((total_packages) * sizeof(char *));

        if (width == 0) /* Allocate space to get the whole line */
        {
            for (i = 0; i < total_packages; i++)
                packages[i] = (char *) malloc((nbytes + 1) * sizeof(char));
        }
        else /* Allocate space for packages with equal length */
        {
            for (i = 0; i < total_packages; i++)
                packages[i] = (char *) malloc((width + 1) * sizeof(char));
        }

        /* Clear iterators */
        i = 0;
        j = 0;

        /* Get correctly the matrix of packages where lines are the number of current package and columns are the content */
        while ((ch = fgetc(output_file)) != EOF)
        {
            packages[i][j] = ch;

            if (width != 0 && j == (width - 1))
            {
                packages[i][width] = '\0';
                j = 0;
                i++;
            }
            else
                j++;
        }

        *cbuffer = packages;
        
        fclose(output_file);
        free(filename);
        free(justname);
    }
    else
        total_packages = -1; /* Case of don't open the archieve -1 is returned */

    return total_packages;
}

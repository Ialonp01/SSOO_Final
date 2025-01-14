#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <time.h>


pthread_mutex_t semaforoLog;
pthread_mutex_t semaforoClientes;
pthread_t condReponedor;
pthread_t cond;
int contadorClientes;
struct Clientes{
    char nombre[12];
    int estado;
};
struct Clientes *clientes;


int main(int argc, char *argv[]){
    if(argc!=3){
        perror("Error");
    }
    int numCajeros=atoi(argv[1]);
    int numMaxClientes=atoi(argv[2]);
    srand(time(NULL));
    //Inicializo vector
    clientes=(struct *Cliente)malloc(numCajeros*sizeOf(struct Cliente));
    pthread_mutex_init(&semaforoClientes, NULL);
    pthread_cond_init(&condReponedor,NULL);
    pthread_cond_init(&cond,NULL);
    pthread_mutex_init(&semaforoLog, NULL);
    for(int i=0;i<numCajeros;i++){
        pthread_t cajero;
        char nombre[10];
        sprintf(nombre, "Cliente%d", i);
        pthread_create(&cajero, NULL, &metCajero, (void*)nombre);
    }
    pthread_t reponedor;
    pthread_create(&reponedor,NULL,&metReponedor,NULL);
    signal(SIGUSR1, nuevoCliente);
    while(1){
        pause();
    }
    return 0;
}

void writeLogMessage(char *id, char *msg){
    //Calculo hora actual
    time_t now =time(0);
    struct tm *tlocal=localtime(&now);
    char stnow[25];
    strftime(stnow, 25, "%d %m %y  %H: %M: %S");

    //Escribo en el log
    logFile=fopen(logFileName, "a");
    fprintf(logFile, "[%s] %s: %s\n", stnow, id, msg);
    fclose(logFile);
}

void nuevoCliente(int signal){
    pthread_mutex_lock(&semaforoClientes);
    for(int i=0;i<sizeOf(clientes);i++){
        if(*(clientes+i)->estado==-1){
            *(clientes+i)->estado=0;
            char nombre[12];
            sprintf(nombre, "Cliente_%d", ++contadorClientes);
            clientes[i].nombre=nombre;
            //Como ya acabé con el uso de la lista 
            pthread_mutex_unlock(&semaforoClientes);
            
            //creo el hilo
            pthread_t cliente;
            pthread_create(&cliente, NULL, &metCliente, (void*)nombre);
            return;
        }
    }
     //Si se sale del for es que no hay hueco por lo tanto no puede entrar
     writeLogMessage("Sistema", "No hay hueco en la cola");

}

int calculaAleatorio(){

}

void *metCliente(void *arg){
    char *id=(char*) arg;
    while(1){
        pthread_mutex_lock(&semaforoClientes);
        int pos=getPosicionCliente(id);
        if(clientes[pos].estado!=0){
            break;
        }
        else{
            pthread_mutex_unlock(&semaforoClientes);
            sleep(10);
            int aleatorio=calculaAleatorio(0,10);
            if(aleatorio<1){
                writeLogMessage(id, "Me cansé de esperar");
                pthread_mutex_lock(&semaforoClientes);
                eliminar(id);
                pthread_mutex_unlock(&semaforoClientes);
                pthread_exit(NULL);
            }
        }
    }
    writeLogMessage(id,"Estoy esperando");
    while(1){
        pthread_mutex_lock(&semaforoClientes);
        int pos=getPosicionCliente(id);
        if(clientes[pos].estado==2){
            pthread_mutex_unlock(&semaforoClientes);
            break;
        }
        pthread_mutex_unlock(&semaforoClientes);
        sleep(2);
    }
    writeLogMessage(id,"Han terminado de atenderme");
    pthread_mutex_lock(&semaforoClientes);
    eliminar(id);
    pthread_mutex_unlock(&semaforoClientes);
}

void *metCajero(void *args){
    //generar identificador
    char *id=(char*) arg;
    //escribir en el log y empezamos a atender
    writeLogMessage(id,"Empezamos a atender")
    //iniciar contador de atendidas
    int contadorClientes=0;
    //buscar cliente para atender
    char *idCliente=NULL;
    while(1){
        pthread_mutex_lock(&semaforoClientes);
        for(int i=0;i<sizeOf(clientes);i++){
            *idCliente=*(cliente+i)->nombre;
            if(*(clientes+i)->estado==0){
                *(clientes+i)->estado=1;
                pthread_mutex_unlock(&semaforoClientes);
                break;
            }
        }
        pthread_mutex_unlock(&semaforoClientes);
        if(idCliente==NULL){
            //Espero a buscar hueco
        }
        else{
            
        }
    }
}

int getPosicionCliente(char *nombre){

}

void eliminar(char* nombre){   
    int pos=getPosicionCliente(nombre);
    int i=0;
    for(i=pos;i<sizeof(clientes)-1;i++){
        clientes[i]=clientes[i+1];
    }
    clientes[i].nombre=NULL;
    clientes[i].estado=-1;
}

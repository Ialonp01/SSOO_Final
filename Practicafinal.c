#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <time.h>


pthread_mutex_t semaforoLog;
pthread_mutex_t semaforoClientes;
pthread_mutex_t mutexReponedor;
pthread_t condReponedor;
pthread_t cond;
pthread_t condCajero;
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
    pthread_mutex_init(&mutexReponedor, NULL);
    pthread_cond_init(&condReponedor,NULL);
    pthread_cond_init(&cond,NULL);
    pthread_cond_init(&condCajero,NULL)
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


 void *calculaAleatorio(int minimo, int maximo){
    return rand()%(maximo-minimo+1)+minimo;

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
    writeLogMessage(id,"Empezamos a atender");
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
        if(idCliente!=NULL){
            //Espero a buscar hueco
            int atencion=calculaAleatorio(1,5);
            char msg[100];
            sprintf("Comienza la atención de %s ", idCliente);
            writeLogMessage(id, msg);
            sleep(atencion);
            int probab=calculaAleatorio(1,100);
            if(probab>95){
                char noAtencion[100];
                sprintf(noAtencion,"%s se va sin comprobar");
                writeLogMessage(id, noAtencion);
            }else if(probab>70){
                pthread_cond_signal(&condReponedor);
                char noAtencion[100];
                sprintf(noAtencion,"%s tiene que esperar al reponedor");
                writeLogMessage(id,noAtencion);
                //Se espera el aviso del reponedor
                pthread_mutex_lock(&mutexReponedor);
                pthread_cond_wait(&cond,&mutexReponedor);
            }
            int precio=calculaAleatorio(1,100);
            if(probab<95){
                char fin[100];
                sprintf(fin,"El cliente %s finaliza su atención con un total de %d euros", id);
                writeLogMessage(id,fin);
            }
            pthread_mutex_lock(&semaforoClientes);
            for(int i=0;i<sizeof(clientes);i++){
                if(strcmp(clientes[i].nombre, idCliente)==0){
                    clientes[i].estado=2;
                    break;
                }
            }
            pthread_mutex_unlock(&semaforoClientes);
            contadorClientes++;
            if(contadorClientes%10==0){
                writeLogMessage(id,"Voy a descansar");
                sleep(20);
            }
        }
    }
}
void *reponedor(void *args){
    while(1){
        writeLogMessage("Reponedor", "Comienza el método");
        pthread_mutex_lock(&mutexReponedor);
        pthread_mutex_wait(&condReponedor,&mutexReponedor);
        writeLogMessage("Reponedor", "Estoy consultando");
        int siesta=calculaAleatorio(1,5);
        sleep(siesta);
        pthread_cond_signal(&condCajero);
    }
}

int getPosicionCliente(char *nombre){
    for(int i = 0; i<contClientes; i++){
        if(strcmp(nombre, cliente[i].nombre)==0){
            return i;
        }
    }
    printf("No existe el cliente.");
    return -1;
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

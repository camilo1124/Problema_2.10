/* Autor: Camilo Andrés Cárdenas Vargas */
/* Grupo 08 */
/* Problema 2.10 - La taqueria de paco */
/* Definiciones externas del modelo de la taqueria */

#include "simlib.h"				/*Uso de la biblioteca simlib*/

#define EVENTO_LLEGADA 			1	/* Llegada de un grupo de clientes al sistema */
#define EVENTO_SALIDA			2	/* Salida de un grupo despues de ser atendido */
#define EVENTO_CERRAR_RESTATURANTE	3	/* El restaurante cierra despues de haber completado un turno*/


#define SAMPST_TIEMPO_ESPERA		1	/* Variable sampst para tiempo promedio en cola*/		
#define SAMPST_TIEMPO_COMIDA		2	/* Variable sampst para tiempo promedio de servicio*/	
#define SAMPST_TAMANO_GRUPO		3	/* Variable sampst para determinar promedio de grupos*/	
#define SAMPST_TIEMPO_ENTRELLEGADA	4	/* Variable sampst para tiempo de llegada entre grupos*/



#define STREAM_INTERARRIVAL		1	/* Stream para el numero aleatorio para el tiempo entre llegadas */
#define STREAM_TAMANO			2	/* Stream para el numeero aleatorio para el tamaño del grupo */
#define STREAM_ORDENES			3	/* Stream para el numero aleatiorio para la cantidad de ordenes*/
#define STREAM_COMIDA			4	/* Stream para el numero aleatiorio para el tiempo para comer */

/* Declaración de variables globales */

int num_mesas,mesas_dos,mesas_cuatro,tam_grupo[4],ordenes_pers[4],precio_orden,costo_orden,
    salario_hora,turno_en_horas,clientes_totales,esperantes_totales,ordenes_totales_turno,max_clientes_cola = -5;
float prob_tam_grupo[4],prob_ordenes_pers[4],prob_tiempo_comida[5],media_llegadas,
      F_prob_tam_grupo[4],F_prob_ordenes_pers[4],F_prob_tiempo_comida[5],tiempo_comida[5],tiempo;
FILE *infile,*outfile;

/* Declaración de funciones propias del programa */ 

void llegada();
void salida();
void report();

int main(){
	infile = fopen("taqueria.in","r");
	outfile = fopen("taqueria.out","w");

	/* Se leen las variables de entrada */

	fscanf(infile, "%d %d %d %d",&precio_orden,&costo_orden,&salario_hora,&turno_en_horas);
	fscanf(infile, "%d %d %d",&mesas_dos,&mesas_cuatro,&num_mesas);
	for (int i = 1; i <= 3; i++){
		fscanf(infile,"%d",&tam_grupo[i]);
	}
	for(int i = 1; i <= 3; i++){
		fscanf(infile,"%f",&prob_tam_grupo[i]); 
	}
	for(int i = 1; i <= 3; i++){
		fscanf(infile,"%f",&F_prob_tam_grupo[i]); 
	}

	for (int i = 1; i <= 3; i++){
		fscanf(infile,"%d",&ordenes_pers[i]);
	}
	for(int i = 1; i <= 3; i++){
		fscanf(infile,"%f",&prob_ordenes_pers[i]); 
	}
	for(int i = 1; i <= 3; i++){
		fscanf(infile,"%f",&F_prob_ordenes_pers[i]); 
	}
	for (int i = 1; i <= 4; i++){
		fscanf(infile,"%f",&tiempo_comida[i]);
	}
	for(int i = 1; i <= 4; i++){
		fscanf(infile,"%f",&prob_tiempo_comida[i]); 
	}
	for(int i = 1; i <= 4; i++){
		fscanf(infile,"%f",&F_prob_tiempo_comida[i]); 
	}
	fscanf(infile, "%f", &media_llegadas);

	/* Encabezado del reporte */

	fprintf(outfile, "Modelo de la taqueria\n\n");
	fprintf(outfile, "Numero de mesas en el restaurante:\t%d\n\n", num_mesas);
	fprintf(outfile, "Numero de mesas para dos:\t\t%d\n\n",mesas_dos);
	fprintf(outfile, "Numero de mesas para cuatro:\t\t%d\n\n",mesas_cuatro);
	fprintf(outfile, "Tiempo medio entre llegadas(min):\t%.3f\n\n", media_llegadas);
	fprintf(outfile, "Costo de una orden:\t\t\t%d\n\n",costo_orden);
	fprintf(outfile, "Precio de una orden:\t\t\t%d\n\n",precio_orden);
	fprintf(outfile, "Turno en horas:\t\t\t%d\n\n",turno_en_horas);
	fprintf(outfile, "Salario de un mesero por hora:\t%d\n\n",salario_hora);
	fprintf(outfile, "____________________________________\n\n");
	fprintf(outfile, "Tamaño del grupo:\t");
	for (int i = 1;i <= 3; i++)fprintf(outfile,"\t%d",tam_grupo[i]);
	fprintf(outfile, "\nProbabilidad:\t\t");
	for (int i = 1;i <= 3; i++)fprintf(outfile,"\t%.3f",prob_tam_grupo[i]); 
	fprintf(outfile, "\n__________________________________\n\n");
	fprintf(outfile, "____________________________________\n\n");
	fprintf(outfile, "Ordenes/persona:\t");
	for (int i = 1;i <= 3; i++)fprintf(outfile,"\t%d",ordenes_pers[i]);
	fprintf(outfile, "\nProbabilidad:\t\t");
	for (int i = 1;i <= 3; i++)fprintf(outfile,"\t%.3f",prob_ordenes_pers[i]); 
	fprintf(outfile, "\n__________________________________\n\n");
	fprintf(outfile, "____________________________________\n\n");
	fprintf(outfile, "Tiempo(min/orden)\t");
	for (int i = 1;i <= 3; i++)fprintf(outfile,"\t%f",tiempo_comida[i]);
	fprintf(outfile, "\nProbabilidad:\t\t");
	for (int i = 1;i <= 3; i++)fprintf(outfile,"\t%.3f",prob_tiempo_comida[i]);
	fprintf(outfile, "\n__________________________________\n\n");


	init_simlib();

	/* numero de atributos por registro */

	maxatr = 6;

	clientes_totales = 0;
	esperantes_totales = 0;
	ordenes_totales_turno = 0;

	/* Programar la primera llegada */

	tiempo = (sim_time + expon(media_llegadas,STREAM_INTERARRIVAL))*100;
	event_schedule(tiempo,EVENTO_LLEGADA);
	sampst(tiempo,SAMPST_TIEMPO_ENTRELLEGADA);
	
	/* Programar la cerrada de la taquieria */
	
	event_schedule((float)(turno_en_horas * 60), EVENTO_CERRAR_RESTATURANTE);
	
	while (list_size[LIST_EVENT] != 0){

		printf("sim_time: %.3f\n",sim_time);

		/* Determinar el siguiente evento */
		timing();
		switch (next_event_type){
			case EVENTO_LLEGADA:
				llegada();
				break;
			case EVENTO_SALIDA:
				salida((int)transfer[3]);
				break;
			case EVENTO_CERRAR_RESTATURANTE:
				event_cancel(EVENTO_LLEGADA);
				break;
		}
	}

	reporte();

	fclose(infile);
	fclose(outfile);

	return 0;

	

}

void llegada(){

	int mesa,primera_mesa,ultima_mesa,ordenes_totales = 0,tipo_cola,tam_grupo_entrante;
        float tiempo_total = 0;

	/* Programar la siguiente llegada */
	tiempo = sim_time + (expon(media_llegadas,STREAM_INTERARRIVAL)*100);
	event_schedule(tiempo,EVENTO_LLEGADA);
	sampst(tiempo,SAMPST_TIEMPO_ENTRELLEGADA);


	/* Determinar el tamaño del grupo entrante */
	tam_grupo_entrante = tam_grupo[random_integer(F_prob_tam_grupo,STREAM_TAMANO)];

	/* Tambien mirar la clase de mesa y el tamaño del grupo */

	if (tam_grupo_entrante <= 2){ 
		primera_mesa = 1;
       		ultima_mesa = mesas_dos;
		tipo_cola = 1;
	}else{ 
		primera_mesa = mesas_dos;
		ultima_mesa = num_mesas;
		tipo_cola = 2;
	}

	/* Si una mesa está libre, comenzar el servicio del grupo */

	for (mesa = primera_mesa ; mesa <= ultima_mesa; mesa++){

		if (list_size[mesa] == 0){

			/* Hacer la mesa ocupada */
			sampst(0.0,SAMPST_TIEMPO_ESPERA);

			for (int persona = 1; persona <= tam_grupo_entrante; persona++){
				ordenes_totales += random_integer(F_prob_ordenes_pers,STREAM_ORDENES);
				ordenes_totales_turno += ordenes_totales;
			}
			for (int num_orden = 1; num_orden <= ordenes_totales; num_orden++){
				tiempo_total += tiempo_comida[random_integer(F_prob_tiempo_comida,STREAM_COMIDA)];
			}
			list_file(FIRST, mesa);
			transfer[4] = tam_grupo_entrante;
			transfer[3] = mesa;
			event_schedule(sim_time + tiempo_total,EVENTO_SALIDA);	

			sampst(tiempo_total,SAMPST_TIEMPO_COMIDA);
			return;
		}

	}

	/* Todas las mesas estan ocupadas, poner el grupo en su respectiva cola */

	transfer[1] = sim_time;
	transfer[4] = tam_grupo_entrante;
	esperantes_totales += tam_grupo_entrante;

	/*Se hace uso de "2 colas" pero solo es una, sin embargo el comportamiento
	 * de un restaurante en la vida real, exige este tipo de programación */

	list_file(FIRST, (num_mesas + tipo_cola));		/* Una cola dependiendo el tipo de grupo */
	list_file(FIRST, (num_mesas + 3));			/* Una cola para todos sirve para las estadisticas */

	/* Evaluar la cantidad de grupos en cola */

	if (max_clientes_cola < list_size[num_mesas + 3]){
		max_clientes_cola = list_size[num_mesas + 3];
	}
	
}

void salida(int mesa){

	clientes_totales += (int)transfer[4];			/* Info sobre los numero de clientes atendidos */
	sampst(transfer[4],SAMPST_TAMANO_GRUPO);		/* Info para el promedio de grupos */

	int ordenes_totales = 0,tipo_cola,tam_grupo_entrante;
        float tiempo_total = 0;

	if (mesa <= mesas_dos){
		tipo_cola = 1;
	}else{
		tipo_cola = 2;
	}
	
	/*Mirar si la cola para la respectiva mesa está vacía */

	if (list_size[num_mesas + tipo_cola] == 0){
		/* Hacer una mesa libre */
		list_remove(FIRST, mesa);

	}else{
		/* Programar un servicio inmediatamente para una mesa */
		list_remove(FIRST, num_mesas + 3); /*Estadistica num max de clientes en la cola */

		list_remove(FIRST, num_mesas + tipo_cola);
		sampst(sim_time - transfer[1], SAMPST_TIEMPO_ESPERA);
		tam_grupo_entrante = (int)transfer[4];
		for (int persona = 1; persona <= tam_grupo_entrante; persona++){
			ordenes_totales += random_integer(F_prob_ordenes_pers,STREAM_ORDENES);
			ordenes_totales_turno += ordenes_totales;
		}
		for (int num_orden = 1; num_orden <= ordenes_totales; num_orden++){
			tiempo_total += tiempo_comida[random_integer(F_prob_tiempo_comida,STREAM_COMIDA)];
			sampst(tiempo_total, SAMPST_TIEMPO_COMIDA);
		}
		transfer[3] = mesa;
		event_schedule(sim_time + tiempo_total,EVENTO_SALIDA);	

	}
}

void reporte(){

	/* Utilidad basandonos solo en el costo de producir una orden y venderla */
	int utilidad_precio_costo = ordenes_totales_turno * (precio_orden - costo_orden);
	/* Utilidad añadiendo el costo de los salarios de los 2 meseros */
	int utilidad_total = utilidad_precio_costo - (salario_hora * turno_en_horas); 
	/* Probabilidad de no encontrar mesa disponible */
	float probabilidad_no_mesa = (float)esperantes_totales/(float)clientes_totales;

	fprintf(outfile,"______________________________________________________\n");
	fprintf(outfile," \nResultados\n\n");
	fprintf(outfile,"______________________________________________________\n");
	fprintf(outfile, "\nUtilidades\n\n");
	fprintf(outfile, " Cantidad de ordenes pedidas durante un turno: %d \n\n", ordenes_totales_turno);
	fprintf(outfile, " Utilidad precio/costo en un turno: %d \n\n", utilidad_precio_costo);
	fprintf(outfile, " Utilidad total en un turno (teniendo en cuenta salarios de 2 meseros): %d \n\n",
			utilidad_total);
	fprintf(outfile,"______________________________________________________\n");
	fprintf(outfile, "\nProbabilidad de NO encontrar mesa disponible\n\n");
	fprintf(outfile, "Clientes totales atendidos: %d\n\n", clientes_totales);
	fprintf(outfile, "Clientes que espareron en cola: %d\n\n", esperantes_totales);
	fprintf(outfile, "Probabilidad de no encontrar mesa: %.3f\n\n", probabilidad_no_mesa);
	fprintf(outfile,"______________________________________________________\n");
	fprintf(outfile, "\nNumero de clientes(grupos) maximo en cola : %d\n\n",max_clientes_cola);
	fprintf(outfile,"_______________________________________________________________________\n\n");
	fprintf(outfile,"\n\nA continuación se hará uso de la función out_sampst\n");
	fprintf(outfile,"Las variables estan acompañados con el numero  sampst correspondiente en la tabla\n");
	fprintf(outfile,"En la tabla prestar atención a los averages que significa promedio\n");
	fprintf(outfile,"\nMedidas de desempeño para esta seccion\n\n");
	fprintf(outfile,"1.Tiempo promedio de espera en cola.\n\n");
	fprintf(outfile,"2.Tiempo promedio de comida por grupo.\n\n");
	fprintf(outfile,"3.Tamaño de grupo promedio.\n\n");
	fprintf(outfile,"4.Tiempo entre llegadas promedio.\n\n");
	out_sampst(outfile, SAMPST_TIEMPO_ESPERA,SAMPST_TIEMPO_ENTRELLEGADA); 
	
}





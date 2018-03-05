#!/usr/bin/python
 
import sys

argumentos = raw_input()

try:
	float(argumentos)
	argumentos = float(argumentos)
	resultado = 1.8*argumentos + 32	
	print "La temperatura en Fahrenheit es: ", resultado
except ValueError:
	print "Por favor, introduzca un numero la proxima vez"

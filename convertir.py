import sys

def convertir_archivo(nombre_archivo):
    with open(nombre_archivo, 'r') as f:
        lineas = [linea.strip() for linea in f if linea.strip() != '']
    
    m, n = map(int, lineas[0].split())
    
    costos = []
    idx = 1
    while len(costos) < n:
        costos.extend(map(int, lineas[idx].split()))
        idx += 1
    
    conjuntos = [set() for _ in range(n)]
    
    for i in range(1, m+1):
        partes = list(map(int, lineas[idx].split()))
        idx += 1
        grado = partes[0]
        columnas = partes[1:]
        for col in columnas:
            conjuntos[col-1].add(i)
    
    # FILTRAR: eliminar conjuntos vacíos
    conjuntos = [s for s in conjuntos if s]
    n = len(conjuntos)  # actualizar el número de conjuntos
    
    universo = set(range(1, m+1))
    
    salida = []
    salida.append(f"Universo = {set_a_string(universo)}")
    lista_conjuntos = ", ".join(set_a_string(s) for s in conjuntos)
    salida.append(f"Conjuntos = [ {lista_conjuntos} ]")
    salida.append("respuesta = ResolverCobertura(Universo, Conjuntos)")
    
    return "\n".join(salida)

def set_a_string(s):
    if not s:
        return "{}"
    elementos = ", ".join(str(x) for x in sorted(s))
    return f"{{{elementos}}}"

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Uso: python convertir_or_library.py archivo.txt")
        sys.exit(1)
    print(convertir_archivo(sys.argv[1]))
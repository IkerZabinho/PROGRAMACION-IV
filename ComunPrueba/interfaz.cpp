#include "Clases.h"
#include "interfaz.h" // Incluimos tu cabecera de interfaz
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>  // Para sprintf
#include <ctime>
#include <cstring>

using namespace std;

namespace GestionONG {

    // ============================================================================
    // 1. GESTIÓN ECONÓMICA DE BENEFICIARIOS
    // ============================================================================

    Beneficiario guardarCondicionesBeneficiario() {
        int correcto;
        int adultos = 0, ninos = 0;
        float sueldos = 0, ayudas = 0, alquiler = 0, suministros = 0, material_escolar = 0, estudios = 0, otros = 0;

        cout << "\n--- DETALLES ECONÓMICOS DEL BENEFICIARIO ---\n";
        cout << "* Responde a la pregunta y pulsa enter para continuar.\n";

        // 1. INTEGRANTES
        do {
            cout << "\n> INTEGRANTES DE LA FAMILIA\n";
            cout << "Número de adultos en casa: ";
            cin >> adultos;
            cout << "Número de niños/as en casa: ";
            cin >> ninos;
            
            cout << "  > ¿Deseas cambiar algún dato de los integrantes? (1: Sí / 0: No): ";
            cin >> correcto;

            if (correcto == 1) cout << "[!] Reintentando integrantes...\n";
        } while (correcto != 0);

        // 2. INGRESOS
        do {
            cout << "\n> INGRESOS\n";
            cout << "Sueldo mensual total: ";
            cin >> sueldos;
            cout << "Otras ayudas/pensiones: ";
            cin >> ayudas;
            
            cout << "  > ¿Deseas cambiar algún dato de los ingresos? (1: Sí / 0: No): ";
            cin >> correcto;

            if (correcto == 1) cout << "[!] Reintentando ingresos...\n";
        } while (correcto != 0);

        // 3. GASTOS
        do {
            cout << "\n> GASTOS\n";
            cout << "Alquiler o hipoteca: ";
            cin >> alquiler;
            cout << "Luz, agua y gas: ";
            cin >> suministros;
            cout << "Material escolar: ";
            cin >> material_escolar;
            cout << "Gastos en estudios: ";
            cin >> estudios;
            cout << "Otros gastos: ";
            cin >> otros;

            cout << "  > ¿Deseas cambiar algún dato de los gastos? (1: Sí / 0: No): ";
            cin >> correcto;

            if (correcto == 1) cout << "[!] Reintentando gastos...\n";
        } while (correcto != 0);

        float ingresos_totales = sueldos + ayudas;
        float gastos_totales = alquiler + suministros + material_escolar + estudios + otros;

        Beneficiario b(0, "", "", "", "", 0, adultos, ninos, ingresos_totales, gastos_totales);
        //b.evaluarBeneficiario();

        return b;
    }

    // ============================================================================
    // 2. SISTEMA DE REGISTRO E INICIO DE SESIÓN
    // ============================================================================

  void registrarUsuario(sqlite3 *db) {
    int rolElegido;
    string nombre, apellidos, nombre_usuario, contrasena;
    
    cout << "\n--- REGISTRO DE NUEVO USUARIO ---\n";
    cout << "Elige tu rol:\n1. Voluntario\n2. Donante\n3. Beneficiario\nRol (1-3): ";
    
    if (!(cin >> rolElegido) || rolElegido < 1 || rolElegido > 3) {
        cout << "Rol no válido.\n";
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    }
    
    TipoUsuario tipo = (TipoUsuario)rolElegido; 

    cout << "Nombre: ";
    cin.ignore(); 
    getline(cin, nombre);

    cout << "Apellidos: ";
    getline(cin, apellidos);

    cout << "Nombre de usuario: ";
    cin >> nombre_usuario;

    cout << "Contraseña: ";
    cin >> contrasena;

    Usuario u(0, nombre, apellidos, nombre_usuario, contrasena, tipo);

    void *datosE = NULL;

    Beneficiario b(0, "", "", "", "", 0, 0, 0, 0.0f, 0.0f);
    Voluntario v(0, "", "", "", "", 0, "");
    Donante d(0, "", "", "", "", 0);

    if (tipo == BENEFICIARIO) {
        b = guardarCondicionesBeneficiario();
        void evaluarBeneficiario(GestionONG::Beneficiario &b);
        b.setNombre(nombre); 
        b.setApellidos(apellidos); 
        b.setNombreUsuario(nombre_usuario); 
        b.setContrasena(contrasena);
        
        datosE = &b;
    } 
    else if (tipo == VOLUNTARIO) {
        v.setNombre(nombre); 
        v.setApellidos(apellidos); 
        v.setNombreUsuario(nombre_usuario); 
        v.setContrasena(contrasena);
        
        datosE = &v;
    }
    else if (tipo == DONANTE) {
        d.setNombre(nombre); 
        d.setApellidos(apellidos); 
        d.setNombreUsuario(nombre_usuario); 
        d.setContrasena(contrasena);
        
        datosE = &d;
    }

    // Inserción en la Base de Datos
    int id_perfil_especifico = Usuario::insertarUsuario(db, u, datosE);

    // Bloque de feedback al usuario final
    if (id_perfil_especifico != -1) {
        
        // Comportamiento diferenciado para la impresión de pantallas de éxito
        if (tipo == BENEFICIARIO) {
            cout << "\n[OK] Registro de Beneficiario completado con éxito.\n";
        } 
        else if (tipo == VOLUNTARIO) {
            cout << "\n[Red] Enviando paquete de registro seguro al servidor por TCP...\n";
            cout << ">>> ¡ÉXITO! ¡Voluntario registrado correctamente! <<<\n";
        } 
        else if (tipo == DONANTE) {
            cout << "\n[OK] Registro de Donante completado con éxito. ¡Gracias por tu apoyo!\n";
        }

    } else {
        cout << "\n[!] ERROR: No se pudo registrar. Puede que el nombre de usuario ya exista.\n";
    }

    cout << "\nPresiona ENTER para volver al menú principal...";
    cin.ignore(10000, '\n');
    cin.get(); 
}

    void iniciarSesion(sqlite3 *db) {
        string user, pass;
        Usuario sesion(0, "", "", "", "", VOLUNTARIO); 

        cout << "\n--- INICIAR SESIÓN ---\n";
        cout << "\nUsuario: ";
        cin >> user;
        cout << "Contraseña: ";
        cin >> pass;

        if (Usuario::comprobarLogin(db, user, pass, &sesion)) {
            int id_perfil = buscarIdEspecifico(db, sesion.getId(), (int)sesion.getTipo());
            
            if (id_perfil != -1) {
                cout << "\n¡Bienvenido, " << sesion.getNombre() << " " << sesion.getApellidos() << "!\n";
                
                menuPrincipal(db, (int)sesion.getTipo(), id_perfil);
                
            } else {
                cout << "Error: No se encontró un perfil asociado a este usuario.\n";
            }
        } else {
            cout << "Error: Usuario o contraseña incorrectos.\n";
        }
    }

    // ============================================================================
    // 3. SECCIÓN DE DONACIONES
    // ============================================================================

    void donarRopa(sqlite3 *db, int id_usuario) {
        Ropa r(0, 0, 0.0f); 
        int respuesta;

        cout << "\n--- REALIZAR DONACIÓN DE ROPA ---\n";
        cout << "Cantidad en kilogramos: ";
        
        if (!(cin >> r.kilos) || r.kilos <= 0) {
            cout << "[!] Error: Debes introducir un peso válido mayor que 0.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }

        cout << "¿Confirmas donar " << r.kilos << " kg de ropa?\n0. No\n1. Sí\nSelección: ";
        cin >> respuesta;

        if (respuesta == 1) {
            if (insertarDonacionRopa(db, r, id_usuario) == 0) {
                cout << "\n[ÉXITO] Tu donación de " << r.kilos << " kg ha sido registrada.\n";
            } else {
                cout << "\n[!] No se pudo completar la donación en la base de datos.\n";
            }
        } else {
            cout << "Operación cancelada.\n";
        }
    }

    void donarDinero(sqlite3 *db, int id_donante) {
        Dinero d(0, 0, 0.0f); 
        int confirmar;

        cout << "\n--- DONACIÓN DE DINERO ---\n";
        cout << "Cantidad: ";
        if (!(cin >> d.cantidad) || d.cantidad <= 0) {
            cout << "Error en cantidad.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }

        cout << "¿Confirmar " << d.cantidad << "€? (1:Si / 0:No): ";
        cin >> confirmar;

        if (confirmar == 1) {
            if (insertarDonacionDinero(db, d, id_donante) == SQLITE_OK) {
                cout << "[EXITO] Guardado.\n";
            }
        }
    }

    void donarComida(sqlite3 *db, int id_donante) {
        Donacion d;
        Comida c(0, CARBOHIDRATOS, 0.0f, 0); 
        int respuesta, seleccion;
        vector<string> nombresCategorias = {"", "Carbohidratos", "Legumbres", "Conservas", "Lacteos"};

        cout << "\n--- REALIZAR DONACION DE COMIDA ---\n";

        do {
            cout << "Seleccione el tipo de alimento:\n";
            for(size_t i = 1; i < nombresCategorias.size(); i++) {
                cout << i << ". " << nombresCategorias[i] << "\n";
            }
            cout << "Seleccion: ";
            
            if (!(cin >> seleccion)) {
                cout << "[!] Error: Introduce un numero.\n";
                cin.clear();
                cin.ignore(10000, '\n');
                seleccion = -1;
                continue;
            }

            if (seleccion < 1 || seleccion > 4)
                cout << "[!] Opcion no valida, elige entre 1 y 4.\n";
        } while (seleccion < 1 || seleccion > 4);

        c.tipo_comida = (TipoComida)seleccion;

        cout << "Cantidad en kilogramos: ";
        if (!(cin >> c.kilos) || c.kilos <= 0) {
            cout << "[!] Error: debes introducir un numero valido mayor que 0.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }

        cout << "¿Confirmas donar " << c.kilos << " kg de " << nombresCategorias[seleccion] << "?\n0. No\n1. Si\nSeleccion: ";
        cin >> respuesta;

        if (respuesta == 1) {
            d.id_usuario = id_donante;
            d.tipoDonacion = COMIDAD; 

            if (insertarDonacionComidaDB(db, d, c) == 0) {
                cout << "\n[EXITO] Donacion registrada correctamente.\n";
                mostrarProximaRecogida(db, 1); 
            }
        } else {
            cout << "Operacion cancelada.\n";
        }
    }

    // ============================================================================
    // 4. PARTICIPACIÓN EN EVENTOS Y TALLERES
    // ============================================================================

    void apuntarseEvento(sqlite3 *db, int id_voluntario) {
        Participacion p;
        p.id_voluntario = id_voluntario;
        sqlite3_stmt *stmt;

        cout << "\n--- APUNTARSE A UN EVENTO ---\n";
        cout << "\nEventos disponibles en los que aún no participas:\n";

        const char *sql_list = 
            "SELECT id_evento, descripcion, fecha_ini, tipo, material FROM Evento "
            "WHERE date(fecha_ini) >= date('now') "
            "AND id_evento NOT IN (SELECT id_evento FROM Participaciones WHERE id_voluntario = ?);";

        if (sqlite3_prepare_v2(db, sql_list, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id_voluntario);

            printf("\n%-4s | %-10s | %-10s | %-18s | %s\n", "ID", "TIPO", "MATERIAL", "FECHA", "DESCRIPCIÓN");
            cout << "------------------------------------------------------------------------------------\n";

            int encontrados = 0;
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                encontrados = 1;
                int id = sqlite3_column_int(stmt, 0);
                const char *desc = (const char*)sqlite3_column_text(stmt, 1);
                const char *fecha = (const char*)sqlite3_column_text(stmt, 2);
                int tipo_int = sqlite3_column_int(stmt, 3);
                int mat_int = sqlite3_column_int(stmt, 4);

                string txtTipo = (tipo_int == 1) ? "Recogida" : "Reparto";
                string txtMat = (mat_int == 1) ? "Ropa" : "Comida";

                printf("%-4d | %-10s | %-10s | %-18s | %s\n", id, txtTipo.c_str(), txtMat.c_str(), fecha, desc ? desc : "");
            }
            
            if (!encontrados) {
                cout << "[INFO] No hay eventos nuevos disponibles para ti en este momento.\n";
            }
            sqlite3_finalize(stmt);
        } else {
            cout << "Error al consultar eventos: " << sqlite3_errmsg(db) << endl;
        }

        cout << "------------------------------------------------------------------------------------\n";
        cout << "Introduce el ID del evento (0 para cancelar): ";
        if (!(cin >> p.id_evento) || p.id_evento <= 0) {
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }

        if (Evento::tieneChoqueDeFechas(db, p.id_voluntario, p.id_evento)) {
            cout << "\n¡ERROR! Ya tienes otro compromiso registrado para ese mismo día.\n";
            return;
        }

        if (Evento::estaEventoLleno(db, p.id_evento)) {
            cout << "\n¡ERROR! El evento ya tiene suficientes voluntarios.\n";
            return;
        }

        const char *sql_ins = "INSERT INTO Participaciones (id_voluntario, id_evento) VALUES (?, ?);";
        if (sqlite3_prepare_v2(db, sql_ins, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, p.id_voluntario);
            sqlite3_bind_int(stmt, 2, p.id_evento);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                cout << "\n[OK] ¡Inscripción realizada con éxito! Gracias por tu colaboración.\n";
            } else {
                cout << "\n[ERROR] No se pudo completar la inscripción: " << sqlite3_errmsg(db) << endl;
            }
            sqlite3_finalize(stmt);
        }
    }

    void apuntarseTaller(sqlite3 *db, int id_beneficiario) {
        sqlite3_stmt *stmt;
        int id_taller;

        cout << "\n--- TALLERES DISPONIBLES ---\n";

        const char *sql_list = "SELECT id_taller, tipo, descripcion FROM Taller "
                               "WHERE id_taller NOT IN (SELECT id_taller FROM Asistencia WHERE id_beneficiario = ?);";

        if (sqlite3_prepare_v2(db, sql_list, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id_beneficiario);
            
            int encontrados = 0;
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                encontrados++;
                cout << "ID: " << sqlite3_column_int(stmt, 0) 
                     << " | Tipo: " << sqlite3_column_int(stmt, 1) 
                     << " | " << sqlite3_column_text(stmt, 2) << "\n";
            }
            sqlite3_finalize(stmt);

            if (encontrados == 0) {
                cout << "No hay talleres nuevos disponibles para ti en este momento.\n";
                return;
            }
        } else {
            cout << "[!] Error al consultar talleres: " << sqlite3_errmsg(db) << endl;
            return;
        }

        cout << "\nIntroduce el ID del taller al que quieres asistir (0 para cancelar): ";
        if (!(cin >> id_taller) || id_taller <= 0) {
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }

        const char *sql_ins = "INSERT INTO Asistencia (id_beneficiario, id_taller) VALUES (?, ?);";
        if (sqlite3_prepare_v2(db, sql_ins, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id_beneficiario);
            sqlite3_bind_int(stmt, 2, id_taller);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                cout << "\n[OK] Te has inscrito correctamente en el taller " << id_taller << ".\n";
            } else {
                cout << "\n[!] Error al inscribirse: " << sqlite3_errmsg(db) << endl;
            }
            sqlite3_finalize(stmt);
        }
    }

    // ============================================================================
    // 5. CREACIÓN DE ENTIDADES (ADMINISTRACIÓN)
    // ============================================================================

    void crearEvento(sqlite3 *db) {
        Evento e; 
        int temp_material;

        cout << "\n--- CREAR EVENTO ---\n";
        cout << "Descripción: ";
        cin.ignore();
        getline(cin, e.descripcion); 

        cout << "Tipo de evento (1 = Ropa, 2 = Comida): ";
        cin >> temp_material;
        e.material = (Material)temp_material;

        cout << "Límite de voluntarios: ";
        cin >> e.lim_voluntarios;

        cout << "Fecha inicio (DD MM AAAA HH MM):\n";
        while (1) {
            if (leer_y_validar_fecha("Introduce fecha inicio: ", &e.fecha_inicio)) break;
            cout << " Error: La fecha de inicio debe ser válida y futura.\n";
        }

        cout << "Fecha final (DD MM AAAA HH MM):\n";
        while (1) {
            if (leer_y_validar_fecha("Introduce fecha final: ", &e.fecha_fin)) {
                if (comparar_fechas(e.fecha_inicio, e.fecha_fin) == 1) break;
                cout << " Error: La fecha final debe ser posterior a la de inicio.\n";
            } else {
                cout << " Error: Formato incorrecto o fecha pasada.\n";
            }
        }

        if (insertarEvento(db, e) == 0) {
            cout << "Evento registrado correctamente en el sistema.\n";
        } else {
            cout << "Hubo un problema al guardar el evento.\n";
        }
    }

    void crearTaller(sqlite3 *db) {
        sqlite3_stmt *stmt;
        Taller t; 
        int id_voluntario, opcion_tipo;
        char f_ini_str[20], f_fin_str[20];

        cout << "\n--- CREAR NUEVO TALLER ---\n";
        cout << "\n--- VOLUNTARIOS DISPONIBLES ---\n";
        
        const char *sql_v = "SELECT v.id_voluntario, u.nombre FROM Voluntarios v "
                            "JOIN Usuarios u ON v.id_usuario = u.id_usuario;";
        
        int hay_voluntarios = 0;
        if (sqlite3_prepare_v2(db, sql_v, -1, &stmt, 0) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                hay_voluntarios = 1;
                cout << "ID: [" << sqlite3_column_int(stmt, 0) << "] | Nombre: " << sqlite3_column_text(stmt, 1) << "\n";
            }
            sqlite3_finalize(stmt);
        }

        if (!hay_voluntarios) {
            cout << "[!] No hay voluntarios registrados. Registra uno antes de crear un taller.\n";
            return;
        }

        cout << "\nIntroduce el ID del voluntario responsable: ";
        if (!(cin >> id_voluntario)) {
            cout << "[!] Error: ID no válido.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }

        cout << "\nSeleccione el tipo de taller:\n1. Cocina\n2. Aprendizaje\n3. Deportes\n: ";
        cin >> opcion_tipo;
        t.tipo = (TipoTaller)opcion_tipo; 

        cout << "Descripción: ";
        cin.ignore();
        getline(cin, t.descripcion);

        cout << "Fecha inicio (DD MM AAAA HH MM):\n";
        while (1) {
            if (leer_y_validar_fecha("Introduce inicio: ", &t.fecha_ini)) break;
            cout << " [!] Error: Fecha inválida.\n";
        }

        cout << "Fecha fin (DD MM AAAA HH MM):\n";
        while (1) {
            if (leer_y_validar_fecha("Introduce fin: ", &t.fecha_fin)) break;
            cout << " [!] Error: Fecha inválida.\n";
        }

        sprintf(f_ini_str, "%04d-%02d-%02d %02d:%02d", t.fecha_ini.anyo, t.fecha_ini.mes, t.fecha_ini.dia, t.fecha_ini.hora, t.fecha_ini.minutos);
        sprintf(f_fin_str, "%04d-%02d-%02d %02d:%02d", t.fecha_fin.anyo, t.fecha_fin.mes, t.fecha_fin.dia, t.fecha_fin.hora, t.fecha_fin.minutos);

        const char *sql_ins = "INSERT INTO Taller (tipo, fecha_ini, fecha_fin, descripcion, id_voluntario) VALUES (?, ?, ?, ?, ?);";

        if (sqlite3_prepare_v2(db, sql_ins, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, (int)t.tipo); 
            sqlite3_bind_text(stmt, 2, f_ini_str, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, f_fin_str, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, t.descripcion.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 5, id_voluntario);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                cout << "\n[OK] Taller creado con éxito.\n";
                int id_taller_recien_creado = (int)sqlite3_last_insert_rowid(db);

                sqlite3_stmt *stmt_rel;
                const char *sql_rel = "INSERT INTO Impartir (id_voluntario, id_taller) VALUES (?, ?);";
                if (sqlite3_prepare_v2(db, sql_rel, -1, &stmt_rel, 0) == SQLITE_OK) {
                    sqlite3_bind_int(stmt_rel, 1, id_voluntario);
                    sqlite3_bind_int(stmt_rel, 2, id_taller_recien_creado);
                    sqlite3_step(stmt_rel);
                    sqlite3_finalize(stmt_rel);
                }
            } else {
                cout << "\n[!] Error SQL: " << sqlite3_errmsg(db) << "\n";
            }
            sqlite3_finalize(stmt);
        }
    }

    // ============================================================================
    // 6. FUNCIONES COMPLEMENTARIAS DE EVENTOS, USUARIOS Y DONACIONES
    // ============================================================================

    void borrarEvento(sqlite3 *db) {
        int id_borrar;
        char sql[200];
        char *error = 0;

        printf("\n--- ELIMINAR EVENTO ---\n");
        listarEventos(db); 

        printf("\nIntroduce el ID del evento a eliminar (0 para cancelar): ");
        if (!(cin >> id_borrar) || id_borrar <= 0) {
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }

        sprintf(sql, "DELETE FROM Evento WHERE id_evento = %d;", id_borrar);

        if (sqlite3_exec(db, sql, 0, 0, &error) == SQLITE_OK) {
            printf("\n[OK] Evento eliminado correctamente.\n");
        } else {
            printf("\n[!] Error al borrar: %s\n", error);
            sqlite3_free(error);
        }
    }

    void listarEventos(sqlite3 *db) {
        sqlite3_stmt *stmt;
        const char *sql = "SELECT id_evento, descripcion, fecha_ini, material, tipo FROM Evento ORDER BY fecha_ini ASC;";

        printf("\n%-5s | %-25s | %-18s | %-10s | %-10s\n", "ID", "DESCRIPCIÓN", "FECHA", "MATERIAL", "TIPO");
        printf("--------------------------------------------------------------------------------------\n");

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            int hay_eventos = 0;
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                hay_eventos = 1;
                int id = sqlite3_column_int(stmt, 0);
                const char *desc = (const char *)sqlite3_column_text(stmt, 1);
                const char *fecha = (const char *)sqlite3_column_text(stmt, 2);
                int mat_int = sqlite3_column_int(stmt, 3);
                int tipo_int = sqlite3_column_int(stmt, 4);

                const char *mat_txt = (mat_int == 0) ? "Ropa" : "Comida";
                const char *tipo_txt = (tipo_int == 0) ? "Recogida" : "Reparto";

                printf("%-5d | %-25s | %-18s | %-10s | %-10s\n", 
                       id, 
                       desc ? desc : "Sin desc.", 
                       fecha ? fecha : "---", 
                       mat_txt, 
                       tipo_txt);
            }

            if (!hay_eventos) {
                printf("No hay eventos registrados en la base de datos.\n");
            }
            sqlite3_finalize(stmt);
        } else {
            printf("[!] Error al listar eventos: %s\n", sqlite3_errmsg(db));
        }
        printf("--------------------------------------------------------------------------------------\n");
    }

    void darBajaUsuario(sqlite3 *db) {
        int id_eliminar;
        int confirmar;

        printf("\n--- DAR DE BAJA USUARIO ---\n");
        listarUsuarios(db); 

        printf("Introduce ID del usuario para eliminar (0 para cancelar): ");
        if (!(cin >> id_eliminar) || id_eliminar <= 0) {
            cin.clear();
            cin.ignore(10000, '\n');
            printf("Operación cancelada.\n");
            return;
        }

        printf("¿Estás seguro de eliminar al usuario %d?\n0. No\n1. Sí\nSelección: ", id_eliminar);
        if (!(cin >> confirmar)) confirmar = 0;

        if (confirmar == 1) {
            if (eliminarUsuarioDB(db, id_eliminar) == 0) {
                printf("\n[ÉXITO] Usuario %d eliminado correctamente.\n", id_eliminar);
            } else {
                printf("\n[!] No se pudo eliminar el usuario.\n");
            }
        } else {
            printf("Operación cancelada.\n");
        }
    }

    void consultarMisEventos(sqlite3 *db, int id_voluntario) {
        sqlite3_stmt *stmt;
        const char *sql_list = 
            "SELECT E.id_evento, E.descripcion, E.fecha_ini, E.tipo, E.material "
            "FROM Evento E "
            "JOIN Participaciones P ON E.id_evento = P.id_evento "
            "WHERE P.id_voluntario = ? AND datetime(E.fecha_ini) >= datetime('now', 'localtime') "
            "ORDER BY E.fecha_ini ASC;";

        printf("\n--- CALENDARIO DE MIS EVENTOS ---\n");
        printf("%-5s | %-12s | %-10s | %-18s | %s\n", "ID", "TIPO", "MATERIAL", "FECHA", "DESCRIPCIÓN");
        printf("------------------------------------------------------------------------------------\n");

        if (sqlite3_prepare_v2(db, sql_list, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id_voluntario);

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmt, 0);
                const char *desc = (const char*)sqlite3_column_text(stmt, 1);
                const char *fecha = (const char*)sqlite3_column_text(stmt, 2);
                int tipo_int = sqlite3_column_int(stmt, 3);
                int mat_int = sqlite3_column_int(stmt, 4);

                const char *txtTipo = (tipo_int == 0) ? "Recogida" : "Reparto";
                const char *txtMat = (mat_int == 0) ? "Ropa" : "Comida";

                printf("%-5d | %-12s | %-10s | %-18s | %s\n", id, txtTipo, txtMat, fecha, desc ? desc : "");
            }
            sqlite3_finalize(stmt);
        } else {
            printf("Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        }
        printf("------------------------------------------------------------------------------------\n");

        int opcion, id_borrar;
        printf("¿Quieres desapuntarte de alguno? (1: Sí / 0: No): ");
        if (!(cin >> opcion)) {
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }

        if (opcion == 1) {
            printf("Introduce el ID del evento: ");
            if (cin >> id_borrar) {
                char sql_del[200];
                sprintf(sql_del, "DELETE FROM Participaciones WHERE id_voluntario = %d AND id_evento = %d;", 
                        id_voluntario, id_borrar);
                
                char *error = 0;
                if (sqlite3_exec(db, sql_del, 0, 0, &error) == SQLITE_OK) {
                    if (sqlite3_changes(db) > 0) printf("\n[OK] Te has desapuntado con éxito.\n");
                    else printf("\n[!] No estabas inscrito en ese evento.\n");
                } else {
                    printf("\nError: %s\n", error);
                    sqlite3_free(error);
                }
            }
        }
    }

    void consultarHistorialEventos(sqlite3 *db, int id_voluntario) {
        sqlite3_stmt *stmt;
        const char *sql = 
            "SELECT E.id_evento, E.descripcion, E.fecha_ini, E.tipo, E.material "
            "FROM Evento E "
            "JOIN Participaciones P ON E.id_evento = P.id_evento "
            "WHERE P.id_voluntario = ? AND datetime(E.fecha_ini) < datetime('now', 'localtime') "
            "ORDER BY E.fecha_ini DESC;";

        printf("\n--- HISTORIAL DE EVENTOS PASADOS ---\n");
        printf("%-5s | %-12s | %-10s | %-18s | %s\n", "ID", "TIPO", "MATERIAL", "FECHA", "DESCRIPCIÓN");
        printf("------------------------------------------------------------------------------------\n");

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id_voluntario);

            int hay_eventos = 0;
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                hay_eventos = 1;
                int id = sqlite3_column_int(stmt, 0);
                const char *desc = (const char*)sqlite3_column_text(stmt, 1);
                const char *fecha = (const char*)sqlite3_column_text(stmt, 2);
                int tipo_int = sqlite3_column_int(stmt, 3);
                int mat_int = sqlite3_column_int(stmt, 4);

                const char *txtTipo = (tipo_int == 0) ? "Recogida" : "Reparto";
                const char *txtMat = (mat_int == 0) ? "Ropa" : "Comida";

                printf("%-5d | %-12s | %-10s | %-18s | %s\n", id, txtTipo, txtMat, fecha, desc ? desc : "");
            }
            
            if (!hay_eventos) {
                printf("\n[INFO] Aún no has participado en eventos anteriores.\n");
            }
            sqlite3_finalize(stmt);
        } else {
            printf("Error SQL: %s\n", sqlite3_errmsg(db));
        }

        printf("------------------------------------------------------------------------------------\n");
        printf("Presione Enter para volver...");
        cin.ignore(10000, '\n');
        cin.get();
    }

    void listarDonaciones(sqlite3 *db, int id_donante) {
        sqlite3_stmt *stmt;
        const char *sql =
            "SELECT d.tipo, r.kilos, c.tipo_comida, c.kilos, din.cantidad, d.fecha "
            "FROM Donaciones d "
            "LEFT JOIN Ropa r ON d.id_donacion = r.id_donacion "
            "LEFT JOIN Comida c ON d.id_donacion = c.id_donacion "
            "LEFT JOIN Dinero din ON d.id_donacion = din.id_donacion "
            "WHERE d.id_donante = ? ORDER BY d.id_donacion DESC;";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
            printf("Error: %s\n", sqlite3_errmsg(db));
            return;
        }

        sqlite3_bind_int(stmt, 1, id_donante);

        printf("\n%-12s | %-45s | %-20s\n", "TIPO", "DETALLES", "FECHA");
        printf("------------------------------------------------------------\n");

        int hay_donaciones = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            hay_donaciones = 1;
            int tipo = sqlite3_column_int(stmt, 0);
            const char *fecha = sqlite3_column_text(stmt, 5)
                                ? (const char *)sqlite3_column_text(stmt, 5)
                                : "Sin fecha";

            switch (tipo) {
                case 2: // COMIDA
                    printf("%-12s | %s - %.2f kg | %s\n",
                           "COMIDA",
                           sqlite3_column_text(stmt, 2)
                               ? (const char *)sqlite3_column_text(stmt, 2)
                               : "Desconocido",
                           sqlite3_column_double(stmt, 3),
                           fecha);
                    break;
                case 1: // DINERO
                    printf("%-12s | Importe: %.2f EUR | %s\n",
                           "DINERO",
                           sqlite3_column_double(stmt, 4),
                           fecha);
                    break;
                case 3: // ROPA
                    printf("%-12s | Ropa variada - %.2f kg | %s\n",
                           "ROPA",
                           sqlite3_column_double(stmt, 1),
                           fecha);  
                    break;
                default:
                    printf("%-12s | Sin detalles específicos | %s\n", "OTROS", fecha);
                    break;
            }
        }

        if (!hay_donaciones) {
            printf("No se han encontrado donaciones.\n");
        }
        printf("--------------------------------------------------------------------\n");

        sqlite3_finalize(stmt);
    }

  void crearEventoMartesAutomatico(sqlite3 *db) {
    sqlite3_stmt *stmt;
    char *error = 0;
    
    const char *sql_check = "SELECT COUNT(*) FROM Evento "
                            "WHERE descripcion = 'Reparto semanal comida' "
                            "AND date(fecha_ini) = date('now', 'weekday 2');";

    int existe = 0;
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            existe = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    if (existe == 0) {
        const char *sql_insert = 
            "INSERT INTO Evento (material, descripcion, fecha_ini, fecha_fin, tipo, lim_voluntarios) "
            "VALUES (1, 'Reparto semanal comida', "
            "strftime('%Y-%m-%d 09:00', 'now', 'weekday 2'), "
            "strftime('%Y-%m-%d 11:00', 'now', 'weekday 2'), "
            "1, 20);"; // 👈 Ahora es una cadena de texto limpia y continua para SQLite
            
        if (sqlite3_exec(db, sql_insert, 0, 0, &error) == SQLITE_OK) {
            printf("[SISTEMA] Evento automático de los martes creado (Reparto semanal comida).\n");
        } else {
            printf("[!] Error al crear evento automático: %s\n", error);
            sqlite3_free(error);
        }
    }
}


    // ============================================================================
    // AUXILIAR: LEER Y VALIDAR FECHA
    // ============================================================================
int Fecha::leer_y_validar_fecha(const string& mensaje, Fecha *f) {
    int correcto = 0;
    do {
        cout << mensaje;
        cout << "\nIntroduce el año (aaaa): ";
        cin >> f->anyo;
        cout << "Introduce el mes (1-12): ";
        cin >> f->mes;
        cout << "Introduce el día (1-31): ";
        cin >> f->dia;
        cout << "Introduce la hora (0-23): ";
        cin >> f->hora;
        f->minutos = 0;
        
        cin.ignore(10000, '\n'); 

        if (f->anyo >= 2026 && f->anyo <= 2100 &&
            f->mes >= 1 && f->mes <= 12 && 
            f->dia >= 1 && f->dia <= 31 && 
            f->hora >= 0 && f->hora <= 23) {
            
            correcto = 1;
        } else {
            cout << "[!] Fecha o hora no válida (comprueba que el año esté entre 2026 y 2100 y que el mes y la hora cumplan los requisitos)" << "\nInténtalo de nuevo.\n";
        }
    } while (!correcto);

    return 1; 
}

    // 3B. FUNCIÓN SUELTA (Para las llamadas viejas de interfaz.cpp que daban error)
    int leer_y_validar_fecha(const string& mensaje, Fecha *f) {
        Fecha aux;
        return aux.leer_y_validar_fecha(mensaje, f);
    }

   // ============================================================================
    // GESTIÓN DE EVENTOS AUTOMÁTICOS Y CONSULTAS DE AYUDA
    // ============================================================================

    void crearEventoJuevesRopaAutomatico(sqlite3 *db) {
        sqlite3_stmt *stmt;
        char *error = 0;
        
        const char *sql_check = "SELECT COUNT(*) FROM Evento "
                                "WHERE descripcion = 'Recogida semanal ropa' "
                                "AND date(fecha_ini) = date('now', 'weekday 4');";

        int existe = 0;
        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                existe = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        if (existe == 0) {
            const char *sql_insert = 
                "INSERT INTO Evento (material, descripcion, fecha_ini, fecha_fin, tipo, lim_voluntarios) "
                "VALUES (0, 'Recogida semanal ropa', "
                "strftime('%Y-%m-%d 10:00', 'now', 'weekday 4'), " 
                "strftime('%Y-%m-%d 13:00', 'now', 'weekday 4'), " 
                "0, 15);";
            
            if (sqlite3_exec(db, sql_insert, 0, 0, &error) == SQLITE_OK) {
                printf("[SISTEMA] Evento automatico de los jueves creado (Recogida semanal ropa).\n");
            } else {
                printf("[!] Error al crear evento ropa: %s\n", error);
                sqlite3_free(error);
            }
        }
    }

    void verProximoRepartoComida(sqlite3 *db) {
        sqlite3_stmt *stmt;
        const char *sql = "SELECT descripcion, fecha_ini, fecha_fin FROM Evento "
                          "WHERE material = 1 AND tipo = 1 "
                          "AND date(fecha_ini) >= date('now') "
                          "ORDER BY fecha_ini ASC LIMIT 1;";

        printf("\n--- PROXIMO REPARTO DE COMIDA ---\n");
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                printf("Evento: %s\n", sqlite3_column_text(stmt, 0));
                printf("Fecha de inicio: %s\n", sqlite3_column_text(stmt, 1));
                printf("Fecha final: %s\n", sqlite3_column_text(stmt, 2));
                printf("\nLos repartos de comida son CADA MARTES desde las 16:00 hasta las 20:00\n");
            } else {
                printf("No hay repartos de comida programados proximamente.\n");
            }
        } else {
            printf("Error al consultar: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
    }

    void verProximoRepartoRopa(sqlite3 *db, int id_beneficiario) {
        sqlite3_stmt *stmt;
        char fecha_corte[11] = "1900-01-01";
        int tiene_registro = 0;
        int id_usuario = -1;

        const char *sql_get_user = "SELECT id_usuario FROM Beneficiario WHERE id_beneficiario = ?;";
        if (sqlite3_prepare_v2(db, sql_get_user, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id_beneficiario);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                id_usuario = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);

        if (id_usuario == -1) {
            printf("\n--- PROXIMO REPARTO DE ROPA ---\n");
            printf("Error: No se encontro el usuario asociado.\n");
            return;
        }

        const char *sql_ultima = "SELECT MAX(e.fecha_ini) FROM Evento e "
                                 "JOIN Participaciones p ON e.id_evento = p.id_evento "
                                 "WHERE p.id_usuario = ? AND e.material = 0 AND e.tipo = 0;";

        if (sqlite3_prepare_v2(db, sql_ultima, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id_usuario);
            if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_text(stmt, 0) != NULL) {
                const char *ultima_fecha = (const char *)sqlite3_column_text(stmt, 0);
                tiene_registro = 1;

                sqlite3_stmt *stmt_calc;
                const char *sql_meses = "SELECT date(?, '+6 months');";
                if (sqlite3_prepare_v2(db, sql_meses, -1, &stmt_calc, 0) == SQLITE_OK) {
                    sqlite3_bind_text(stmt_calc, 1, ultima_fecha, -1, SQLITE_STATIC);
                    if (sqlite3_step(stmt_calc) == SQLITE_ROW) {
                        strcpy(fecha_corte, (const char *)sqlite3_column_text(stmt_calc, 0));
                    }
                    sqlite3_finalize(stmt_calc);
                }
            }
        }
        sqlite3_finalize(stmt);

        const char *sql_proximo = "SELECT descripcion, fecha_ini, fecha_fin FROM Evento "
                                  "WHERE material = 0 AND tipo = 0 "
                                  "AND date(fecha_ini) >= date('now') ";

        char sql_con_filtro[500];
        if (tiene_registro) {
            sprintf(sql_con_filtro, "%s AND date(fecha_ini) >= date('%s') ", sql_proximo, fecha_corte);
        } else {
            sprintf(sql_con_filtro, "%s", sql_proximo);
        }
        strcat(sql_con_filtro, "ORDER BY fecha_ini ASC LIMIT 1;");

        printf("\n--- PROXIMO REPARTO DE ROPA ---\n");
        if (sqlite3_prepare_v2(db, sql_con_filtro, -1, &stmt, 0) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                printf("Evento: %s\n", sqlite3_column_text(stmt, 0));
                printf("Fecha inicio: %s\n", sqlite3_column_text(stmt, 1));
                printf("Fecha fin: %s\n", sqlite3_column_text(stmt, 2));
                if (tiene_registro) {
                    printf("Tu proximo reparto estara disponible a partir de: %s\n", fecha_corte);
                    printf("(Deben pasar 6 meses desde tu ultima recogida)\n");
                }
            } else {
                if (tiene_registro) {
                    printf("Aun no puedes recoger ropa. Fecha disponible a partir de: %s\n", fecha_corte);
                } else {
                    printf("No hay repartos de ropa programados en este momento.\n");
                }
            }
        } else {
            printf("Error al consultar: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
    }

    void verTalleresProximos(sqlite3 *db) {
        sqlite3_stmt *stmt;
        const char *sql = "SELECT tipo, descripcion, fecha_ini, fecha_fin FROM Taller "
                          "WHERE date(fecha_ini) >= date('now') "
                          "ORDER BY fecha_ini ASC;";

        printf("\n--- LISTA DE PROXIMOS TALLERES ---\n");
        printf("%-15s | %-25s | %-17s | %-17s\n", "TIPO", "DESCRIPCION", "INICIO", "FIN");
        printf("----------------------------------------------------------------------------------\n");

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            int hay_talleres = 0;
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                hay_talleres = 1;
                const char *tipo = (const char *)sqlite3_column_text(stmt, 0);
                const char *desc = (const char *)sqlite3_column_text(stmt, 1);
                const char *ini  = (const char *)sqlite3_column_text(stmt, 2);
                const char *fin  = (const char *)sqlite3_column_text(stmt, 3);

                printf("%-15s | %-25s | %-17s | %-17s\n", 
                       tipo ? tipo : "General", 
                       desc ? desc : "Sin descripcion", 
                       ini  ? ini  : "---", 
                       fin  ? fin  : "---");
            }
            if (!hay_talleres) {
                printf("No hay talleres programados para los proximos dias.\n");
            }
        } else {
            printf("Error al consultar la tabla Taller: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
        printf("----------------------------------------------------------------------------------\n");
    }

    // ============================================================================
    // FUNCIONES AUXILIARES MATEMÁTICAS Y DE BASE DE DATOS
    // ============================================================================

    float calcularAyudaDinero(Beneficiario b) {
        float renta = b.getIngresos() - b.getGastos();
        return abs(int(renta)) + 50.0f; 
    }

    void mostrarAyudaComida(Beneficiario b) {
        float totalArrozPasta = (b.getNumAdultos() * 1.0f) + (b.getNumNinos() * 0.75f);
        float totalLegumbres = (b.getNumAdultos() + b.getNumNinos()) * 0.5f;
        float totalLeche = (b.getNumAdultos() * 2.0f) + (b.getNumNinos() * 4.0f);
        int totalConservas = (b.getNumAdultos() * 3) + (b.getNumNinos() * 2);

        printf("\n[ALIMENTACION SEMANAL]");  
        printf("\n > Arroz/Pasta:        %.2f kg", totalArrozPasta);
        printf("\n > Legumbres:          %.2f kg", totalLegumbres);
        printf("\n > Leche:              %.0f litros", totalLeche);
        printf("\n > Conservas:          %d latas\n", totalConservas);
    }

    void mostrarAyudaRopa(Beneficiario b) {
        int camNinos = b.getNumNinos() * 3;
        int panNinos = b.getNumNinos() * 2;
        int sudNinos = b.getNumNinos() * 1;
        int camAdultos = b.getNumAdultos() * 2;
        int panAdultos = b.getNumAdultos() * 1;

        printf("\n[VESTIMENTA SEMESTRAL]"); 
        if (b.getNumNinos() > 0) {
            printf("\n > NINOS/AS: %d camisetas, %d pantalones, %d sudaderas", camNinos, panNinos, sudNinos);
        }
        if (b.getNumAdultos() > 0) {
            printf("\n > ADULTOS: %d camisetas, %d pantalones", camAdultos, panAdultos);
        }
        printf("\n");
    }

    int actualizarDatosBeneficiario(sqlite3 *db, int id_beneficiario, Beneficiario b) {
        sqlite3_stmt *stmt;
        const char *sql = "UPDATE Beneficiario SET ingresos = ?, gastos = ?, num_adultos = ?, num_ninos = ? WHERE id_beneficiario = ?;";
        int rc = 0;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_double(stmt, 1, b.getIngresos());
            sqlite3_bind_double(stmt, 2, b.getGastos());
            sqlite3_bind_int(stmt, 3, b.getNumAdultos());
            sqlite3_bind_int(stmt, 4, b.getNumNinos());
            sqlite3_bind_int(stmt, 5, id_beneficiario);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                rc = 1;
            }
            sqlite3_finalize(stmt);
        }
        return rc;
    }

    // Tu función de menú principal adaptada con las lógicas unificadas
    void menuPrincipal(sqlite3 *db, int tipo, int id_perfil) {
        int opcion;
        do {
            printf("\n======= MENU PRINCIPAL =======");
            if (tipo == VOLUNTARIO) {
                printf("\n1. Apuntarse a un evento");
                printf("\n2. Consultar calendario de mis eventos");
                printf("\n3. Consultar historial de mi voluntariado");
            }
            else if (tipo == DONANTE) {
                printf("\n1. Realizar donacion de dinero");
                printf("\n2. Realizar donacion de comida");
                printf("\n3. Realizar donacion de ropa");
                printf("\n4. Consultar historial de mis donaciones");
            }
            else if (tipo == BENEFICIARIO) {
                printf("\n1. Cambiar condiciones");
                printf("\n2. Consultar horarios para recoger ayudas");
                printf("\n3. Ver proximos talleres");
            }

            
            printf("\n0. Cerrar sesion");
            printf("\nSeleccione una opcion: ");
            scanf("%d", &opcion);

            switch(opcion) {
                case 1:
                    if(tipo == VOLUNTARIO) apuntarseEvento(db, id_perfil);
                    else if (tipo == DONANTE) donarDinero(db, id_perfil);
                    else if (tipo == BENEFICIARIO) {
                        Beneficiario b_actualizada = guardarCondicionesBeneficiario();
                        if (actualizarDatosBeneficiario(db, id_perfil, b_actualizada)) {
                            printf("\n---------------------------------------------------------");
                            printf("\n[SISTEMA] Tus condiciones se han actualizado correctamente.\n");
                        }
                    }
                    break;

                case 2:
                    if(tipo == VOLUNTARIO) consultarMisEventos(db, id_perfil);
                    else if(tipo == DONANTE) donarComida(db, id_perfil);
                    else if(tipo == BENEFICIARIO) {
                        printf("\nCONSULTAR HORARIOS DE AYUDAS\n");
                        verProximoRepartoComida(db);
                        verProximoRepartoRopa(db, id_perfil);
                    }
                    break;

                case 3:
                    if(tipo == VOLUNTARIO) consultarHistorialEventos(db, id_perfil);
                    else if(tipo == DONANTE) donarRopa(db, id_perfil);
                    else if(tipo == BENEFICIARIO) verTalleresProximos(db);
                    break;
               
                case 4:
                    if (tipo == DONANTE) listarDonaciones(db, id_perfil);
                    break;
            }
        } while (opcion != 0);
    }


    void verProximaRecogidaRopa(sqlite3 *db) {
    sqlite3_stmt *stmt;
    
    // Consultamos los eventos futuros de tipo recogida de ropa (material = 0, tipo = 0)
    // Mostramos los próximos 6 eventos programados
    string sql = "SELECT descripcion, fecha_ini, fecha_fin, lim_voluntarios FROM Evento "
                 "WHERE material = 0 AND tipo = 0 "
                 "AND date(fecha_ini) >= date('now') "
                 "ORDER BY fecha_ini ASC LIMIT 6;";

    printf("\n--- PROXIMAS RECOGIDAS DE ROPA (PARA VOLUNTARIOS) ---\n");
    printf("%-25s | %-17s | %-17s | %-12s\n", "DESCRIPCION", "INICIO", "FIN", "MAX. VOLUNT");
    printf("----------------------------------------------------------------------------------\n");

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
        int hay_eventos = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            hay_eventos = 1;
            const char *desc = (const char *)sqlite3_column_text(stmt, 0);
            const char *ini  = (const char *)sqlite3_column_text(stmt, 1);
            const char *fin  = (const char *)sqlite3_column_text(stmt, 2);
            int limite       = sqlite3_column_int(stmt, 3);

            printf("%-25s | %-17s | %-17s | %d voluntarios\n", 
                   desc ? desc : "Recogida Ropa", 
                   ini  ? ini  : "---", 
                   fin  ? fin  : "---", 
                   limite);
        }
        
        if (!hay_eventos) {
            printf("No hay tareas de recogida de ropa programadas próximamente.\n");
        }
    } else {
        printf("Error al consultar las recogidas: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    printf("----------------------------------------------------------------------------------\n");
}
void registrarRecogidaRopaAdmin(sqlite3 *db) {
    sqlite3_stmt *stmt;
    int id_beneficiario, id_evento;
    
    printf("\n--- LISTA DE BENEFICIARIOS ---\n");
    const char *sql_benef = "SELECT b.id_beneficiario, u.nombre, u.apellidos "
                             "FROM Beneficiario b JOIN Usuarios u ON b.id_usuario = u.id_usuario;";
    if (sqlite3_prepare_v2(db, sql_benef, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("ID: %d - %s %s\n", sqlite3_column_int(stmt, 0), 
                   sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2));
        }
    }
    sqlite3_finalize(stmt);
    
    printf("Introduce el ID del beneficiario: ");
    scanf("%d", &id_beneficiario);
    
    printf("\n--- EVENTOS DE ROPA FUTUROS ---\n");
    const char *sql_eventos = "SELECT id_evento, descripcion, fecha_ini FROM Evento "
                              "WHERE material = 0 AND tipo = 1 AND date(fecha_ini) >= date('now') "
                              "ORDER BY fecha_ini ASC;";
    if (sqlite3_prepare_v2(db, sql_eventos, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("ID: %d - %s - %s\n", sqlite3_column_int(stmt, 0), 
                   sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2));
        }
    }
    sqlite3_finalize(stmt);
    
    printf("Introduce el ID del evento de ropa: ");
    scanf("%d", &id_evento);

    const char *sql = "INSERT INTO RecogidaRopa (id_beneficiario, id_evento, fecha_recogida) VALUES (?, ?, date('now'));";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_beneficiario);
        sqlite3_bind_int(stmt, 2, id_evento);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("\n[OK] Recogida registrada con éxito.\n");
        } else {
            printf("\n[!] Error: Puede que esa recogida ya esté registrada.\n");
        }
        sqlite3_finalize(stmt);
    }
}


    int buscarIdEspecifico(sqlite3 *db, int id_usuario, int tipoUsuario) {
        return id_usuario; 
    }

    void listarUsuarios(sqlite3 *db) {
        cout << "\n[Cliente] Solicitando lista de usuarios al servidor...\n";
    }

    int eliminarUsuarioDB(sqlite3 *db, int id) {
        return 0;
    }

    void mostrarProximaRecogida(sqlite3 *db, int tipo) {
        cout << "\n[Cliente] Consultando próximas recogidas en el servidor...\n";
    }
// 1. MÉTODO DE LA CLASE (Para interfaz_Admin.cpp)
    int Fecha::es_bisiesto(int anyo) {
        return ((anyo % 4 == 0 && anyo % 100 != 0) || (anyo % 400 == 0));
    }

    // 1B. FUNCIÓN SUELTA (Para las llamadas viejas de interfaz.cpp)
    int es_bisiesto(int anyo) {
        Fecha aux;
        return aux.es_bisiesto(anyo);
    }

    int Fecha::comparar_fechas(const Fecha& f1, const Fecha& f2) {
// 1. Comparar Años
        if (f1.anyo < f2.anyo) return -1;
        if (f1.anyo > f2.anyo) return 1;

        // 2. Si el año es igual, comparar Meses
        if (f1.mes < f2.mes) return -1;
        if (f1.mes > f2.mes) return 1;

        // 3. Si el mes es igual, comparar Días
        if (f1.dia < f2.dia) return -1;
        if (f1.dia > f2.dia) return 1;

        // 4. Si el día es igual, ¡comparamos las Horas! (Aquí estaba tu fallo)
        if (f1.hora < f2.hora) return -1;
        if (f1.hora > f2.hora) return 1;

        // 5. Si la hora es igual, comparar Minutos
        if (f1.minutos < f2.minutos) return -1;
        if (f1.minutos > f2.minutos) return 1;
        
        return 0; 
    }

    // 2B. FUNCIÓN SUELTA (Para las llamadas viejas de interfaz.cpp)
    int comparar_fechas(const Fecha& f1, const Fecha& f2) {
        Fecha aux;
        return aux.comparar_fechas(f1, f2);
    }

    int insertarDonacionRopa(sqlite3* db, const Ropa& r, int id_usuario) {
        // El cliente no hace esto en local, se hará en el servidor por red.
        return 0;
    }

    int insertarDonacionDinero(sqlite3* db, const Dinero& d, int id_donante) {
        // El cliente no hace esto en local, se hará en el servidor por red.
        return 0;
    }

    int insertarDonacionComidaDB(sqlite3* db, const Donacion& d, const Comida& c) {
        // El cliente no hace esto en local, se hará en el servidor por red.
        return 0;
    }

    int insertarEvento(sqlite3* db, const Evento& e) {
        // El cliente no hace esto en local, se hará en el servidor por red.
        return 0;
    }

    
} // Fin del namespace GestionONG
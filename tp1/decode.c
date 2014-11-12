#include <sstream>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

string url_decode(const string &value)
{
    ostringstream url; // String do tipo output stream string (facilita
                       // bastante a manipulação de dados em uma stream
                       // string)

    url << hex; // Alterar base para hexadecimal
    unsigned int AnsiHexa; // inteiro que recebera um valor de 0 a 255
                           // que sera representado como caractere padrao
                           // ANSI.

    // Iteracao para decodificacao da string passada por parametro
    for (unsigned int i = 0; i < value.length(); i++)
    {
      // Compara caractere, um a um, ate que ache uma codificacao do tipo
      // percent-encode com um '%' precedendo o codigo em hexadecimal
      if (int(value[i]) == '%')
      {
        // Converte o codigo em hexadecimal para inteiro
        sscanf(value.substr(i+1, 2).c_str(), "%x", &AnsiHexa);

        // Adquire o caractere equivalente indicado pela variavel AnsiHexa
        url << static_cast<char>(AnsiHexa);

        // Avança dois caracteres da string passada por parametro, pois
        // o percent-encode ja foi analisado
        i = i + 2;
      }
      else
        url << value[i];
    }

  return url.str(); // Retorna url decodificada
}

int main(int argc, char* argv[])
{
    ifstream input_file;  // para tratar as operacoes de abrir, escrever, fechar
                          // do arquivo de entrada
    ofstream output_file; // para tratar as operacoes de abrir, escrever, fechar
                          // do arquivo de entrada
    string string_url;    // string que recebera as url's do arquivo de entrada

    // Verifica quantidade de parametros passados por comando via terminal
    if(argc == 3)
    {
        // Abertura de arquivos
        input_file.open(argv[1]);
        output_file.open(argv[2]);

        // Verificacao se arquivos foram abertos
        if(input_file.is_open() && output_file.is_open())
        {
            while(true)
            {
                input_file >> string_url; // Busca do arquivo de entrada uma url
                output_file << url_decode(string_url); // escreve no arquivo de saida
                                                       // com string decodificada

                // Iteracao acontece ate que o arquivo de entrada chegue no final (eof)
                // Enquanto falso, acrescenta-se uma linha no arquivo de saida para uma
                // nova url ser escrita
                if(input_file.eof())
                    break;
                else
                    output_file << endl;
            }

            // Fechamento de Arquivos
            input_file.close();
            output_file.close();
        }

        return 0;
    }
    else
        cout << "Usage: dencode <input_file> <output_file>" << endl;
}

#include <sstream>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

string url_encode(const string &value)
{
    ostringstream url; // String do tipo output stream string (facilita
                       // bastante a manipulação de dados em uma stream
                       // string)
    url << hex; // Alterar base para hexadecimal

    // Iteracao para codificacao da string passada por parametro
    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {

        string::value_type ch = (*i); //A cada iteracao, obtemos um caractere por vez

        // Se caractere for um caractere fora da lista de especiais
        // ele eh escrito em url normalmente. Caso seja um caractere
        // especial ele é convertido na forma percent-encode
        if (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~')
            url << ch;
        else
            url << '%' << int((unsigned char) ch);
    }

    return url.str(); // Retorna url codificada
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
                output_file << url_encode(string_url); // escreve no arquivo de saida
                                                       // com string codificada

                // Iteracao acontece ate que o arquivo de entrada chegue no final (eof)
                // Enquanto falso, acrescenta-se uma linha no arquivo de saida para uma
                // nova url ser escrita
                if(input_file.eof())
                    break;
                else
                    output_file << endl;
            }

            // Fechamento de arquivos
            input_file.close();
            output_file.close();
        }

        return 0;
    }
    else
        cout << "Usage: encode <input_file> <output_file>" << endl;
}

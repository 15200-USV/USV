# USV - https://github.com/15200-USV/USV.git
Repositório de projeto USV - Rumo à Noruega
Claro! Aqui tens um **guia passo a passo simples** para qualquer colega que **ainda não tem GitHub** ou **não tem o repositório no computador**, para poder começar a colaborar no projeto convosco usando Git e GitHub.

---

## ✅ INSTRUÇÕES PARA COMEÇAR A TRABALHAR NO CÓDIGO (para novos colegas)

---

### 🧱 **Parte 1: Criar Conta GitHub (se ainda não tiver)**

1. Vai a 👉 [https://github.com/join](https://github.com/join)
2. Cria uma conta (usa o e-mail institucional, se possível)
3. Verifica o e-mail
4. Envia o teu **nome de utilizador do GitHub** para o colega que gere o repositório (para te dar permissões)

---

### 🔐 **Parte 2: Instalar o Git**

1. Vai a 👉 [https://git-scm.com](https://git-scm.com)
2. Clica em **"Download for Windows"**
3. Corre o instalador
4. Na parte **"Adjusting your PATH environment"**, seleciona:
   ✔️ **"Git from the command line and also from 3rd-party software"**
5. Termina a instalação

🧪 Verificação:

Abre o terminal (CMD ou PowerShell) e escreve:

```bash
git --version
```

Se aparecer algo como `git version 2.xx.x`, está pronto.

---

### 🗂️ **Parte 3: Clonar o repositório para o PC**

1. Escolhe uma pasta onde queres guardar o projeto (por ex: "Documentos\Projetos")
2. Abre o terminal nessa pasta (Shift + Botão direito → “Abrir terminal aqui”)
3. Escreve o comando:

```bash
git clone https://github.com/15200-USV/USV.git
```

4. Entra na pasta:

```bash
cd USV
```

Agora tens o projeto no teu computador. Podes abrir os ficheiros no Arduino IDE e trabalhar.

---

### 🔁 **Parte 4: Colaborar com os colegas**

Sempre que fores trabalhar no código:

1. **Atualiza o projeto:**

```bash
git pull origin main
```

2. **Faz alterações nos ficheiros**

3. **Guarda e envia as alterações:**

```bash
git add .
git commit -m "Mensagem clara sobre o que alteraste"
git push origin main
```

---

### ⚠️ Notas importantes:

* **Faz sempre `git pull` antes de começares a mexer nos ficheiros**
* Evita editar ficheiros ao mesmo tempo que outro colega
* Se aparecerem conflitos, Git vai-te pedir para escolher manualmente que parte do código fica

---

Queres que te gere um ficheiro `.txt` ou `.pdf` com estas instruções para poderes partilhar com o grupo?


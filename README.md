<div>
	<img src="client/icone/logo_transparent copy.png" width="25%" height="25%" align="right">
	<img src="client/icone/Testo_cropped.png" width="70%" height="70%" align="center">
</div>

## Collaborative Text Editor
The need to work in a group without the co-presence of the actors in the same physical space, pushes towards the creation of ever more effective support systems for cooperative work. For example, Google provides the Docs suite, through which it's possible to edit, in a cooperative and distributed way, different kind of documents (texts, spreadsheets, presentations) and able to scale large numbers of contemporary users: the solution is based on a set of centralized servers that manage traffic to and from individual clients and release the logic necessary to ensure the concurrent operations' correctness.

Programming in C++ we have developed a <b>cooperative text editor</b> which allow simultaneously editing the document to a set of users, guaranteeing the correct result of the insert and edit operations done by different users, regardless of the order in which they are performed (commutativity) and that repeated deletions lead to the same result (idempotence).

## Installation
To install C(++)operative Editor, download and install on your Operating System the QT SDK  [http://qt-project.org/downloads](http://qt-project.org/downloads). After that, follow these step:

1. Open the shell (CTRL+ALT+T) and position yourself on the directory where the file .run is located.

2. Make runnable the file:
   `sudo chmod +x nome-file.run`

3. Run the script:
	 `/nome-file.run`

4. Follow the showed istructions, selecting the directory where install the framework.

5. Running ``/server/create_db.sql`` create the database ``user.db``.

6. Clone the project:

`git clone https://github.com/VitoTassielli/CollaborativeEditor.git`

7. Open QtCreator and select the project folder.

## Guide
Opened the application, there is a *Login* pop-up:

<img src="client/immagini/login2.png" width="40%" height="40%" align="center">

Here you have to set your credentials. If it's your first time, it's necessary to create a new profile, so clink on *Sign Up*:

<img src="client/immagini/signup2.png" width="40%" height="40%" align="center">

After the *Login*:

<img src="client/immagini/open3.png" width="60%" height="60%" align="center">

On the left there are a list of locally files which you can open or choose to open a shared document thanks to its *URI*: if the link is corret the shared file will show up on the left list. Otherwise you can create a new document from scratch.

Opened the file:

<img src="client/immagini/main2.png" width="80%" height="80%" align="center">

On the top menu there are the main operations: create a new document, open a document, many text options (bold, italics, underlined, allignment, ...), changes' history and logout. It's showed the current document's *URI* to invite other people. 
On top right there is the current profile's icon and, if othere users are online on the same document they will show up with their icon (after three online users it's showed a incremental counter and clicking it there is the completed online users list).

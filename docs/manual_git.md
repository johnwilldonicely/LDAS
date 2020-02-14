################################################################################
# SETTING UP A LOCAL COPY OF THE REMOTE REPOSITORY
cd /opt
sudo mkdir LDAS
sudo chown huxprog LDAS
sudo chgrp huxprog LDAS
cd LDAS


################################################################################
# INSTALLING GIT 
...on Fedora/RedHat  
	$ yum install git -y 

...on Ubuntu...  
	$ sudo apt-get install git

## Setting up a remote SSH key
- does key already exist?
	$ ls -al ~/.ssh/
- add a key linked to your email id_rsa
	$ ssh-keygen -t rsa -b 4096 -C "username@email.com"
- check the ssh agent is running
	$ eval "$(ssh-agent -s)"
- add the key to the agent
	$ ssh-add ~/.ssh/id_rsa
- install xclip for cutting & pasteing the key
	$ sudo dnf install xclip
- copy the key
	$ xclip -sel clip < ~/.ssh/id_rsa.pub 

> go to the GitHub repo  
	- Settings (user-settings sidebar)  
		- SSH and GPG keys  
			- New SSH key or Add SSH key  
				- add descriptive title  
				- paste key  
				- save and update profile  

- test if SSH over the HTTPS port is possible
 	$ ssh -T -p 443 git@ssh.github.com

		Hi username! You've successfully authenticated, but GitHub does not
		provide shell access


################################################################################
# CLONING AN EXISTING REPO - requires SSH setup as above

	$ cd [directory_where_all_projects_are_kept]]
        $ git clone http://github.com/johnwilldonicely/LDAS.git
	$ cd LDAS



################################################################################
# INITIAL CONFIGURATION 

	$ cd [project_deirectory]
	$ git init
	$ git config --global user.email "[your_email@provider.com]"
	$ git config --global user.name "[username]"
	$ git config --local user.name "[username@machine]"
	$ git config --global credential.helper 'cache --timeout=3600' 
		- remember username and password for 1 hour

## add directories or files to be ignored to the .gitignore file

	$ echo "bin/" >> .gitignore
	$ echo "backups/" >> .gitignore
	$ echo source/temp_xs-progcompile.log >> .gitignore

## add remote directory

	$ git remote add origin https://github.com/[copmpany]/[repo].git

## specify URL for SSH-key authentication 

- Verify which remotes are using SSH  

	$ git remote -v

- Visit your repository on the web and select the "Clone" button in the upper right  
	- select SSH and copy the new SSH URL  
        - update the URL  

	$ git remote set-url origin [new SSH URL] 
- example....
	$ git remote set-url origin git@github.com:myname/myrepo1.git

## make sure output is in colour
- add this to the git config file:
<pre>
[color]
  diff = auto
  status = auto
  branch = auto
  interactive = auto
  ui = true
  pager = true
</pre>

## list general information on the local repository
	$ git config --list


################################################################################
# RETRIEVE THE REPOSITORY 

	$ git pull origin master


################################################################################
# GIT COMMANDS 

## pull remote master to the local repository

	$ xs-proggit1 pull

	...or...

	$ git pull origin master

## push local repository to remote master

	$ xs-proggit1 push -m "[message]"

	...or...  

	$ git add --all 		# add all changes, new files, & physical removals
	$ git commit -m "message " -a 	# the -a flag adds & removes for "known" files
	$ git push -u origin master


## listing changes 

- summarize local changes (committed in green, otherwise in red)
	$ git status

- list untracked files as well 
	$ git ls-files --others --exclude-standard 

- show 3 most recent commits
	$ git log -3
	... this will show the commit hash-key at the top of each commit

- view the change history of a file
	$ git log -p filename

- compare comitted local changes with remote master
	$ xs-proggit1 diff
	...or...
	$ git fetch origin master
	$ git diff --summary FETCH_HEAD

- compare a particular files(s) with committed changes
	$ git diff [filename]

- show the difference between two commits for a given file
	$ git diff [--options] <commit> <commit> [--] [<path>...]
	...example... 
	$ commit1=f82bc57c3d6120f3fed0cf3dd904be8b07472eb5
	$ commit2=df1875fe3542efced55f7bcda3bacbe3687a7f51
	$ git diff $commit1 $commit2 -- xe-template.c 


## undoing changes 

- remove files you don't want added
	$ rm -f 

- remove files/folders from the header AND physically
	$ git rm -f 

- remove files from the header but leave the physical file in place
	$ git rm --cached 

- undo changes to a file before a commit is made
	$ git checkout [filename]

- undo added but uncommitted changes 
	$ git reset HEAD [filename]
	$ git checkout -- [filename]

- undo committed changes 
	$ git checkout origin/master [filename]

- restore files to a previous commit (referenced by hash-key)
	$ git checkout [hash-key] -- [file1] [file2] ...etc...

- undo recent commits
	1. $ git commit -m "Something terribly misguided"
	2. $ git reset --soft HEAD~
	3. edit files as necessary
	4. $ git add --all.
	5. $ git commit -c ORIG_HEAD

- restore a hopelessly corrupted local repository using the remote master
	1. $ git fetch --all
	2. $ git reset --hard origin/master

- fix most recent commit message
	$ git commit --amend

## restoring the remote master to a previous commit using a local repository

1. show most recent commits
	$ git reflog

2. use one of these reset options: 
 	-  reset to a given commit 
	$ git reset --hard [commit]
	- reset to the commit just BEFORE a given one 
	$ git reset --hard [commit]^
	- ignore the last commit
	$ git reset --hard HEAD^

3. push the reset to the remote repository
	$ git push -f origin master


## stashing changes 
- temporarily stash most recent changes
	$ git stash
- recover the stashed changes
	$ git stash pop
- if you want to preserve the state of files (staged vs. working), use
	$ git stash apply --index



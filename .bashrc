indent_files()
{
	local files=`find -name '*.h' -o -name '*.c'`
	if [ "$files" ]
	then
		indent -linux -l100 $files
	else
		echo 'No such file' >&2
	fi
}

git_info()
{
	local branch
	if branch=`git branch 2> /dev/null`
	then
		branch=`echo "$branch" | sed -n 's/^* //p'`
		echo "($branch `git rev-parse --short HEAD`)"
	fi
}

PS1='\[\e[1;34m\]\w \[\e[1;31m\]`git_info`\[\e[m\]\$ '
LANGUAGE=en_US.UTF-8

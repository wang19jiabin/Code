git_info()
{
	local commit
	commit=`git rev-parse --short HEAD 2> /dev/null`
	[ $? -ne 0 ] && return
	local branch=`git branch | sed -n 's/^* //p'`
	echo "($branch $commit)"
}

PS1='\[\e[1;34m\]\w \[\e[1;31m\]`git_info`\[\e[m\]\$ '
LANGUAGE=en_US.UTF-8

alias indent-files="find -name '*.h' -o -name '*.c' -exec indent -linux -l100 {} +"

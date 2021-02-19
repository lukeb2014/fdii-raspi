Flare Director II Raspberry Pi Code Repository
==============================================

This repository encapsulates the code for the Raspberry Pi.

Quick reference:

Download the repo to a folder:
>> git clone https://github.com/lukeb2014/fdii-raspi

Create a new branch:
>> git b <name_of_branch>

Checkout (start working on) a branch, noting the default is `master`:
>> git checkout <name_of_branch>

Update local code from the online "remote" version:
>> git pull

Add changes to a new potential code version:
>> git add .

Commit changes (makes code push-able)
>> git commit -m "created a new commit"

Push changes to remote
>> git push origin <name_of_branch>

Typically, code should not be pushed directly to the `master` branch. Instead, develop a block of code in a separate branch and perform a "merge request" later. Do merge requests on github.com.

Merge requests combine two branches, resolving any conflicting changes in the code.

## .gitignore

Keep this file updated with any build directories and such. This prevents the repository from becoming gargantuan.

Folders: `/my_folder/**`
Files: `/path_to_file/random_build_file.jar`
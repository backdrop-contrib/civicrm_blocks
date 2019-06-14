# CiviCRM Blocks

This is currently a proof of concept module that borrows heavily from the core
CiviCRM Wordpress integration and the Backdrop core blocks code to allow a user
to insert a CiviCRM Contribution page into a Backdrop Layout via a block.

## Instructions

- Make sure CiviCRM is installed and you have one or more active Contribution Pages
set up.
- Install this module as usual
- Add the "CiviCRM Contribution Page" block to a region in a Layout
- Select an active Contribution page
- Adjust title, visibility settings, etc. as usual

When you view that Layout, if visibility settings on the block allow, it should
pull the CiviCRM contribution page directly in to the Layout in question.
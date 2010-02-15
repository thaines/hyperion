Cyclops dynamically links to 3 LGPL librarys when they are first required:

- gtk. (GIMP Tool Kit.) (www.gtk.org)
- DevIL. (Developers Image Library.) (openil.sourceforge.net)
- libgphoto2 (www.gphoto.org) (Not under Windows. Well, it trys, but linking to something that doesn't exist is  doomed enterprise.)

Frankly, the LGPL could be a lot better as to how to handle this situation. However, the premise of the relevant sections is to allow someone to swap out the LGPL library for something else/a newer version. Hence I have included here the source code that does the actual dynamic linking, which is sufficient to do as such. Whilst I can be certain I have matched the spirit of the LPGL in doing this I haven't the foggiest when it comes to an actual legal interpretation. But then, I probably broke the law when I got up today (A lie-in would of been equally litigious.). Seems we live in a society where the law does not matter, what matters is if the policeman you meet got up on the wrong side of bed, the person who sees you do something thinks its wrong or you piss off someone with more money than you. And people claim we have moved on since the middle ages (Ok, slight exaggeration, but it wouldn't be a good rant otherwise.) Regardless, if you think I could do better just e-mail me.

Rant over,
Tom


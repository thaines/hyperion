//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


#include "sur_test/main.h"

//------------------------------------------------------------------------------
int main()
{
 using namespace eos;
 
 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();
 con << "Starting...\n";


 // Create a mesh, run a load of editting operations on it and save it out, so 
 // we can check its correct...
  str::TokenTable tt;
  sur::Mesh mesh(&tt);


  // Make a cube...
   sur::Vertex v[2][2][2];
   v[0][0][0] = mesh.NewVertex(bs::Vert(-1.0,-1.0,-1.0));
   v[0][0][1] = mesh.NewVertex(bs::Vert(-1.0,-1.0,1.0));
   v[0][1][0] = mesh.NewVertex(bs::Vert(-1.0,1.0,-1.0));
   v[0][1][1] = mesh.NewVertex(bs::Vert(-1.0,1.0,1.0));
   v[1][0][0] = mesh.NewVertex(bs::Vert(1.0,-1.0,-1.0));
   v[1][0][1] = mesh.NewVertex(bs::Vert(1.0,-1.0,1.0));
   v[1][1][0] = mesh.NewVertex(bs::Vert(1.0,1.0,-1.0));
   v[1][1][1] = mesh.NewVertex(bs::Vert(1.0,1.0,1.0));
   
   sur::Face f[3][2];
   f[0][0] = mesh.NewFace(v[0][0][0],v[0][0][1],v[0][1][1],v[0][1][0]);
   f[0][1] = mesh.NewFace(v[1][0][0],v[1][1][0],v[1][1][1],v[1][0][1]);
   
   f[1][0] = mesh.NewFace(v[0][0][0],v[1][0][0],v[1][0][1],v[0][0][1]);
   f[1][1] = mesh.NewFace(v[0][1][0],v[0][1][1],v[1][1][1],v[1][1][0]);
   
   f[2][0] = mesh.NewFace(v[0][0][0],v[0][1][0],v[1][1][0],v[1][0][0]);
   f[2][1] = mesh.NewFace(v[0][0][1],v[1][0][1],v[1][1][1],v[0][1][1]);


  // Manually subdivide a face into 4 smaller faces...
   // Remove the face and its suporting edges...
    ds::Array<sur::Edge> adj(4);
    f[0][0].GetEdges(adj);
    mesh.Del(f[0][0]);
    for (nat32 i=0;i<adj.Size();i++) mesh.Del(adj[i]);

   // Create more vertices...
    sur::Vertex v2[3][3];
    v2[0][0] = v[0][0][0];
    v2[0][1] = mesh.NewVertex(bs::Vert(-1.0,-1.0,0.0));
    v2[0][2] = v[0][0][1];
    
    v2[1][0] = mesh.NewVertex(bs::Vert(-1.0,0.0,-1.0));
    v2[1][1] = mesh.NewVertex(bs::Vert(-1.5,0.0,0.0));
    v2[1][2] = mesh.NewVertex(bs::Vert(-1.0,0.0,1.0));
    
    v2[2][0] = v[0][1][0];
    v2[2][1] = mesh.NewVertex(bs::Vert(-1.0,1.0,0.0));
    v2[2][2] = v[0][1][1];
   
   // Create conecting faces, as 5 vertex faces, only do two of 'em...
    ds::Array<sur::Vertex> mvf(5);

    mvf[0] = v2[0][2];
    mvf[1] = v2[0][1];
    mvf[2] = v2[0][0];
    mvf[3] = v[1][0][0];
    mvf[4] = v[1][0][1];
    mesh.NewFace(mvf);
    
    mvf[0] = v2[2][0];
    mvf[1] = v2[2][1];
    mvf[2] = v2[2][2];
    mvf[3] = v[1][1][1];
    mvf[4] = v[1][1][0];
    mesh.NewFace(mvf);
   
   // Create the 4 shiny new faces...
    mesh.NewFace(v2[0][0],v2[0][1],v2[1][1],v2[1][0]);
    mesh.NewFace(v2[0][1],v2[0][2],v2[1][2],v2[1][1]);
    sur::Face toDie = mesh.NewFace(v2[1][0],v2[1][1],v2[2][1],v2[2][0]);
    mesh.NewFace(v2[1][1],v2[1][2],v2[2][2],v2[2][1]);


  // Make the mesh non-manifold...
    mesh.NewFace(v2[1][0],v2[1][1],v2[1][2]);


  // Move a vertex, reverse a face, delete another face...
   v2[0][0].Pos() += bs::Vert(-0.2,-0.2,-0.2);
   f[0][1].Reverse();
   mesh.Del(toDie);


  // Add some properties to the mesh...
  {
   nat32 natIni = 9;
   real32 realIni = 9.0;
   
   mesh.AddVertProp("turnip",natIni);
   mesh.AddEdgeProp("cabbage",realIni);
   mesh.AddFaceProp("turnip",natIni);
   mesh.AddFaceProp("cabbage",realIni);
   
   mesh.Commit();
  }


  // Create some new geometry...
  {
   sur::Vertex q[2][2];
   q[0][0] = mesh.NewVertex(bs::Vert(2.0,-1.0,-1.0));
   q[0][1] = mesh.NewVertex(bs::Vert(2.0,-1.0,1.0));
   q[1][0] = mesh.NewVertex(bs::Vert(2.0,1.0,-1.0));
   q[1][1] = mesh.NewVertex(bs::Vert(2.0,1.0,1.0));
   
   mesh.NewFace(q[0][0],q[0][1],q[1][1],q[1][0]);
  }


  // Save...
  {
   file::Wavefront wf;
   mesh.Store(wf);
   wf.Save("cube.obj",true);
  }


  // Save as a svt file...
   svt::Core core(tt);
   svt::Node * asSvt = mesh.AsSvt(core);
   svt::Save("cube.svt",asSvt,true);


  // Reload the svt file and save its obj to check for consistancy there...
   sur::Mesh mesh2(&tt);
   if (mesh2.FromSvt(asSvt))
   {
    file::Wavefront wf;
    mesh2.Store(wf);
    wf.Save("cube-2.obj",true);
   }
   else con << "Failed to re-load from svt.\n";
   
  
  // Subdivide mesh2, then save out the result, twice...
  {
   sur::Mesh * sd1 = sur::Subdivide(mesh2);
   {
    file::Wavefront wf;
    sd1->Store(wf);
    wf.Save("cube-3.obj",true);
   }
   sur::Mesh * sd2 = sur::Subdivide(*sd1);
   {
    file::Wavefront wf;
    sd2->Store(wf);
    wf.Save("cube-4.obj",true);
   }
    
   delete sd1;
  }


 delete asSvt;
 con << "Done.\n";
 return 0;
}

//------------------------------------------------------------------------------

// TaskTab.cpp : implementation file
//

#include "stdafx.h"
#include "AssignDialog.h"
#include "TasksTab.h"
#include "ClassToFileUtils.h"

#include "Task.h"
#include < string >
#include < sstream >
#include < iostream >

using std::cout;
using std::endl;
using std::string;
using std::ostringstream;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CTabOne dialog


TasksTab::TasksTab(CWnd* pParent /*=NULL*/)
	: CDialog(TasksTab::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabOne)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void TasksTab::SetPersonnelContainer( vector < Personnel* > *aPersonnelContainer ) {
  personnelContainer = aPersonnelContainer;
}

void TasksTab::SetTaskContainer( vector < Task* > *aTaskContainer ) {
  taskContainer = aTaskContainer;
}


void TasksTab::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control( pDX, IDC_TASK_TREE, m_Tree );
	DDX_Control( pDX, IDC_NAME_EDIT, m_NameEdit );
	DDX_Control( pDX, IDC_DESCR_EDIT, m_DescriptionEdit );
	DDX_Control( pDX, IDC_MH_EDIT, m_MHEdit );
	DDX_Control( pDX, IDC_SUBMH_EDIT, m_SUBMHEdit );
	DDX_Control( pDX, IDC_ASSIGNED_LIST, m_AssignList );
	DDX_Text( pDX, IDC_NAME_EDIT, m_NameValue );
	DDX_Text( pDX, IDC_DESCR_EDIT, m_DescriptionValue );
	DDX_Text( pDX, IDC_MH_EDIT, m_MHValue );
	DDX_Text( pDX, IDC_SUBMH_EDIT, m_SUBMHValue );
	
}


BEGIN_MESSAGE_MAP(TasksTab, CDialog)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TASK_TREE, &TasksTab::OnTvnSelchangedTaskTree)
	ON_BN_CLICKED(IDC_BUTTON_DELETE2, &TasksTab::OnBnClickedButtonDelete2)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_RCLICK, IDC_TASK_TREE, &TasksTab::OnNMRclickTaskTree)
	ON_BN_CLICKED(IDC_BUTTON_NEW, &TasksTab::OnBnClickedButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_EDIT2, &TasksTab::OnBnClickedButtonEdit2)
	ON_BN_CLICKED(IDC_ADDSTAFF, &TasksTab::OnBnClickedAddstaff)
	ON_BN_CLICKED(IDC_REMOVESTAFF, &TasksTab::OnBnClickedRemovestaff)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabOne message handlers
BOOL TasksTab::OnInitDialog()
{
	CDialog::OnInitDialog();

	ReloadTaskTree();

	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void TasksTab::insertChildTasks( Task *parentTask, HTREEITEM parentNode) {	
	vector < Task* > taskVector = parentTask->getSubtasks();
	vector < Task* > :: iterator taskIterator;
	for(taskIterator = taskVector.begin(); 
        taskIterator != taskVector.end();
        taskIterator ++)
	{
		HTREEITEM treeItem;
		Task* subTask = *taskIterator;
		
		treeItem = m_Tree.InsertItem( "", parentNode );
		m_Tree.SetItemText( treeItem, subTask->getName().c_str() );
		m_Tree.SetItemData( treeItem, DWORD_PTR( subTask ));
		insertChildTasks( subTask, treeItem );		
	}
}

void TasksTab::OnTvnSelchangedTaskTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	
	DWORD_PTR dw = m_Tree.GetItemData( hItem ); //get item data
	Task* ItemData = (Task*)dw; //CAST item data
	if (ItemData!=NULL)
	{
		m_NameValue = ItemData->getName().c_str();
		m_DescriptionValue = ItemData->getDescription().c_str();
		m_MHValue.Format( "%d", ItemData->getManhours() );
		m_SUBMHValue.Format( "%d", ItemData->getSubtasksManhours() );
		UpdateData(FALSE);
		for( int i = m_AssignList.GetCount(); i > 0 ; i -- ) {
			m_AssignList.DeleteString( i - 1 );
		}
		vector < Personnel* > &staff = ItemData->getAssignedStaff();
		if( !staff.empty()) {
			vector < Personnel* > :: iterator personnelIterator;
			for(personnelIterator = staff.begin(); 
				personnelIterator != staff.end();
				personnelIterator ++)
			{		
				Personnel* pers = *personnelIterator;
				m_AssignList.InsertString( m_AssignList.GetCount(), pers->getName().c_str() );
				m_AssignList.SetItemData(m_AssignList.GetCount()-1, DWORD_PTR( pers ));
			}
		}
	}

	*pResult = 0;
}

void TasksTab::OnBnClickedButtonDelete2()
{	
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	if( !hItem ) return;


	DWORD_PTR dw = m_Tree.GetItemData( hItem ); //get item data
	Task* ItemData = (Task*)dw;
	if( ItemData != NULL ) {					
		if( ItemData->getParent() ) {
			ItemData->getParent()->removeTask( ItemData );			
		} else {
			vector < Task* > :: iterator taskIterator;
			for(taskIterator = taskContainer->begin(); 
				taskIterator != taskContainer->end();
				taskIterator ++)
			{			
				Task* subTask = *taskIterator;
				if( subTask == ItemData ) {
					taskContainer->erase( taskIterator );
					break;
				}
			}
		}
		delete ItemData;
	}

	ReloadTaskTree();
}
void TasksTab::CleanTaskTree() {
	m_Tree.DeleteAllItems();
}

void TasksTab::ReloadTaskTree() {

	m_Tree.DeleteAllItems();

	vector < Task* > :: iterator taskIterator;
	for(taskIterator = taskContainer->begin(); 
        taskIterator != taskContainer->end();
        taskIterator ++)
	{
		HTREEITEM treeItem;		
		Task* subTask = *taskIterator;
		
		treeItem = m_Tree.InsertItem( "", TVI_ROOT );
		m_Tree.SetItemText( treeItem, subTask->getName().c_str() );
		m_Tree.SetItemData( treeItem, DWORD_PTR( subTask ));	

		insertChildTasks( subTask, treeItem );		
	}

	for( int i = m_AssignList.GetCount(); i > 0 ; i -- ) {
		m_AssignList.DeleteString( i - 1 );
	}
	m_NameValue = "";
	m_DescriptionValue = "";
	m_MHValue = "";
	m_SUBMHValue = "";
	UpdateData(FALSE);	

	return;
}
void TasksTab::OnDestroy()
{
	CDialog::OnDestroy();

	vector < Task* > :: iterator taskIterator;
	for(taskIterator = taskContainer->begin(); 
        taskIterator != taskContainer->end();
        taskIterator ++)
	{		
		Task* subTask = *taskIterator;
		delete subTask;		
	}

	delete taskContainer;
}

void TasksTab::OnNMRclickTaskTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	ReloadTaskTree();
	*pResult = 0;
}

void TasksTab::OnBnClickedButtonNew()
{
	Task *newTask = new Task();
	newTask->setName( "Unnamed Task" );
	newTask->setDescription( "Unnamed Task Description" );

	HTREEITEM hItem = m_Tree.GetSelectedItem();
	if( !hItem ) {
		taskContainer->push_back( newTask );
	} else {
		DWORD_PTR dw = m_Tree.GetItemData( hItem ); //get item data
		Task* ItemData = (Task*)dw;
		if( ItemData != NULL ) {
			ItemData->addTask( newTask );
		}
	}	

	ReloadTaskTree();
}

void TasksTab::OnBnClickedButtonEdit2()
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	if( !hItem ) return;

	DWORD_PTR dw = m_Tree.GetItemData( hItem ); 
	Task* itemData = (Task*)dw;
	if( itemData != NULL ) {					
		UpdateData(TRUE);
		
		itemData->setName( m_NameValue );
		itemData->setDescription( m_DescriptionValue );
		itemData->setManhours( atoi( m_MHValue ) );
		itemData->clearStaff();
		for( int i=0; i < m_AssignList.GetCount(); i++ ) {
			DWORD_PTR dw = m_AssignList.GetItemData( i );
			if( dw != NULL ) {				
				Personnel* staff = (Personnel*)dw;
				itemData->addStaff( staff );				
			}
		}
		
		UpdateData( FALSE );
	}

	ReloadTaskTree();
}

void TasksTab::OnBnClickedAddstaff()
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	if( !hItem ) return;

	AssignDialog dialog;
	
	dialog.setPersonnelContainer( personnelContainer );
	
	INT_PTR nResponse = dialog.DoModal();
	if (nResponse == IDOK)
	{
		vector <Personnel *> :: iterator personnelIterator;
		for(personnelIterator = dialog.getResult()->begin(); 
			personnelIterator != dialog.getResult()->end();
			personnelIterator ++)
		{		
			Personnel* pers = *personnelIterator;
			m_AssignList.InsertString( m_AssignList.GetCount(), pers->getName().c_str() );
			m_AssignList.SetItemData( m_AssignList.GetCount()-1, DWORD_PTR( pers ) );
		}
	}
	else if (nResponse == IDCANCEL)
	{
		
	}
}

void TasksTab::OnBnClickedRemovestaff()
{
	HTREEITEM hItem = m_Tree.GetSelectedItem();
	if( !hItem ) return;

	if( m_AssignList.GetCurSel() == -1 ) {
		return;
	}
	m_AssignList.DeleteString( m_AssignList.GetCurSel() );
}
